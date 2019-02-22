#include "stdafx_shared.h"
#include "pragma/file_formats/wad.h"
#include <pragma/console/conout.h>
#include <mathutil/uquat.h>
#include "pragma/model/animation/fanim.h"

Animation *FWAD::ReadData(unsigned short version,VFilePtr f)
{
	m_file = f;
	Animation *anim = new Animation;
	//unsigned short ver = Read<unsigned short>();
	//Con::cout<<"Animation Version: "<<ver<<Con::endl;

	auto activity = Activity::Invalid;
	if(version >= 0x0013)
	{
		auto activityName = ReadString();
		auto id = Animation::GetActivityEnumRegister().RegisterEnum(activityName);
		activity = (id != util::EnumRegister::InvalidEnum) ? static_cast<Activity>(id) : Activity::Invalid;
	}
	else
		activity = static_cast<Activity>(Read<unsigned short>());
	anim->SetActivity(activity);
	unsigned char activityWeight = Read<unsigned char>();
	anim->SetActivityWeight(activityWeight);

	auto flags = static_cast<FAnim>(Read<unsigned int>());
	anim->SetFlags(flags);

	bool bMoveX = ((flags &FAnim::MoveX) == FAnim::MoveX) ? true : false;
	bool bMoveZ = ((flags &FAnim::MoveZ) == FAnim::MoveZ) ? true : false;
	bool bHasMovement = bMoveX || bMoveZ;

	unsigned int fps = Read<unsigned int>();
	anim->SetFPS(static_cast<unsigned char>(fps));

	if(version >= 0x0007)
	{
		auto min = Read<Vector3>();
		auto max = Read<Vector3>();
		anim->SetRenderBounds(min,max);
	}

	bool bHasFadeIn = Read<bool>();
	if(bHasFadeIn == true)
	{
		float fadeIn = Read<float>();
		anim->SetFadeInTime(fadeIn);
	}

	bool bHasFadeOut = Read<bool>();
	if(bHasFadeOut == true)
	{
		float fadeOut = Read<float>();
		anim->SetFadeOutTime(fadeOut);
	}

	unsigned int numBones = Read<unsigned int>();
	anim->ReserveBoneIds(anim->GetBoneCount() +numBones);
	for(unsigned int i=0;i<numBones;i++)
	{
		unsigned int boneID = Read<unsigned int>();
		anim->AddBoneId(boneID);
	}

	if(version >= 0x0012)
	{
		auto bHasWeights = Read<bool>();
		if(bHasWeights == true)
		{
			auto &weights = anim->GetBoneWeights();
			weights.resize(numBones);
			f->Read(weights.data(),weights.size() *sizeof(weights.front()));
		}
	}

	bool bHasBlendController = Read<bool>();
	if(bHasBlendController == true)
	{
		int controller = Read<int>();
		AnimationBlendController *blend = anim->SetBlendController(controller);
		char numTransitions = Read<char>();
		for(char i=0;i<numTransitions;i++)
		{
			unsigned int animation = Read<unsigned int>();
			int transition = Read<int>();
			blend->transitions.push_back(AnimationBlendControllerTransition());
			AnimationBlendControllerTransition &t = blend->transitions.back();
			t.animation = animation +1; // Account for reference pose
			t.transition = transition;
		}
	}
	
	unsigned int numFrames = Read<unsigned int>();
	for(unsigned int i=0;i<numFrames;i++)
	{
		auto frame = std::make_shared<Frame>(numBones);
		for(unsigned int j=0;j<numBones;j++)
		{
			auto orientation = uquat::identity(); // TODO: Can't use glm::quat here for some reason
			for(unsigned char k=0;k<4;k++)
				orientation[k] = Read<float>();
			Vector3 pos;
			for(unsigned char k=0;k<3;k++)
				pos[k] = Read<float>();
			frame->SetBonePosition(j,pos);
			frame->SetBoneOrientation(j,orientation);
			/*Matx4x3 mat;
			//Mat4x3 mat;
			//Mat
			for(int x=0;x<4;x++)
			{
				for(int y=0;y<3;y++)
					mat[x][y] = Read<float>();
			}
			frame->SetBoneMatrix(j,mat);*/
		}
		unsigned short numEvents = Read<unsigned short>();
		for(unsigned short j=0;j<numEvents;j++)
		{
			AnimationEvent *ev = new AnimationEvent;
			if(version >= 0x0013)
			{
				auto name = ReadString();
				auto id = Animation::GetEventEnumRegister().RegisterEnum(name);
				ev->eventID = (id != util::EnumRegister::InvalidEnum) ? static_cast<AnimationEvent::Type>(id) : AnimationEvent::Type::Invalid;
			}
			else
				ev->eventID = static_cast<AnimationEvent::Type>(Read<unsigned short>());
			unsigned char numParams = Read<unsigned char>();
			for(unsigned char k=0;k<numParams;k++)
			{
				std::string param = ReadString();
				ev->arguments.push_back(param);
			}
			anim->AddEvent(i,ev);
		}
		if(bHasMovement == true)
		{
			Vector2 move(0,0);
			if(bMoveX == true)
				move.x = Read<float>();
			if(bMoveZ == true)
				move.y = Read<float>();
			if(version < 0x0009)
				move *= static_cast<float>(fps);
			frame->SetMoveOffset(move);
		}
		anim->AddFrame(frame);
	}
	//fclose(m_file);
	return anim;
}

Animation *FWAD::Load(unsigned short version,const char *animation)
{
	std::string pathCache(animation);
	std::transform(pathCache.begin(),pathCache.end(),pathCache.begin(),::tolower);

	std::string path = "models\\";
	path += animation;
	const char *cPath = path.c_str();
	auto f = FileManager::OpenFile(cPath,"rb");
	if(f == NULL)
	{
		Con::cout<<"WARNING: Unable to open animation '"<<animation<<"': File not found!"<<Con::endl;
		return NULL;
	}
	Animation *anim = ReadData(version,f);
	return anim;
}
