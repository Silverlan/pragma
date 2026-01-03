// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :assets.file_formats.legacy_weave_animation_data;

std::shared_ptr<pragma::animation::Animation> FWAD::ReadData(unsigned short mdlVersion, ufile::IFile &f)
{
	uint32_t animVersion = 0;
	if(mdlVersion >= 33) {
		animVersion = f.Read<uint32_t>();
		auto offset = f.Tell();
		auto len = f.Read<uint64_t>();
		if(PRAGMA_ANIMATION_VERSION > animVersion || animVersion < 1) {
			f.Seek(offset + len);
			return nullptr;
		}
	}
	auto anim = pragma::animation::Animation::Create();
	//unsigned short ver = Read<unsigned short>();
	//Con::COUT<<"Animation Version: "<<ver<<Con::endl;

	auto activity = pragma::Activity::Invalid;
	if(mdlVersion >= 0x0013) {
		auto activityName = f.ReadString();
		auto id = pragma::animation::Animation::GetActivityEnumRegister().RegisterEnum(activityName);
		activity = (id != pragma::util::INVALID_ENUM) ? static_cast<pragma::Activity>(id) : pragma::Activity::Invalid;
	}
	else
		activity = static_cast<pragma::Activity>(f.Read<unsigned short>());
	anim->SetActivity(activity);
	unsigned char activityWeight = f.Read<unsigned char>();
	anim->SetActivityWeight(activityWeight);

	auto flags = static_cast<pragma::FAnim>(f.Read<unsigned int>());
	anim->SetFlags(flags);

	bool bMoveX = ((flags & pragma::FAnim::MoveX) == pragma::FAnim::MoveX) ? true : false;
	bool bMoveZ = ((flags & pragma::FAnim::MoveZ) == pragma::FAnim::MoveZ) ? true : false;
	bool bHasMovement = bMoveX || bMoveZ;

	unsigned int fps = f.Read<unsigned int>();
	anim->SetFPS(static_cast<unsigned char>(fps));

	if(mdlVersion >= 0x0007) {
		auto min = f.Read<Vector3>();
		auto max = f.Read<Vector3>();
		anim->SetRenderBounds(min, max);
	}

	bool bHasFadeIn = f.Read<bool>();
	if(bHasFadeIn == true) {
		float fadeIn = f.Read<float>();
		anim->SetFadeInTime(fadeIn);
	}

	bool bHasFadeOut = f.Read<bool>();
	if(bHasFadeOut == true) {
		float fadeOut = f.Read<float>();
		anim->SetFadeOutTime(fadeOut);
	}

	unsigned int numBones = f.Read<unsigned int>();
	anim->ReserveBoneIds(anim->GetBoneCount() + numBones);
	for(unsigned int i = 0; i < numBones; i++) {
		unsigned int boneID = f.Read<unsigned int>();
		anim->AddBoneId(boneID);
	}

	if(mdlVersion >= 0x0012) {
		auto bHasWeights = f.Read<bool>();
		if(bHasWeights == true) {
			auto &weights = anim->GetBoneWeights();
			weights.resize(numBones);
			f.Read(weights.data(), weights.size() * sizeof(weights.front()));
		}
	}

	auto hasBlendController = f.Read<bool>();
	if(hasBlendController) {
		int controller = f.Read<int>();
		auto &blend = anim->SetBlendController(controller);
		char numTransitions = f.Read<char>();
		for(char i = 0; i < numTransitions; i++) {
			unsigned int animation = f.Read<unsigned int>();
			auto transition = (mdlVersion >= 29) ? f.Read<float>() : static_cast<float>(f.Read<int>());
			blend.transitions.push_back(AnimationBlendControllerTransition());
			AnimationBlendControllerTransition &t = blend.transitions.back();
			t.animation = animation + 1; // Account for reference pose
			t.transition = transition;
		}

		if(mdlVersion >= 29) {
			blend.animationPostBlendController = f.Read<int32_t>();
			blend.animationPostBlendTarget = f.Read<int32_t>();
		}
	}

	unsigned int numFrames = f.Read<unsigned int>();
	for(unsigned int i = 0; i < numFrames; i++) {
		auto frame = Frame::Create(numBones);
		for(unsigned int j = 0; j < numBones; j++) {
			auto orientation = uquat::identity(); // TODO: Can't use glm::quat here for some reason
			for(unsigned char k = 0; k < 4; k++)
				orientation[k] = f.Read<float>();
			Vector3 pos;
			for(unsigned char k = 0; k < 3; k++)
				pos[k] = f.Read<float>();
			frame->SetBonePosition(j, pos);
			frame->SetBoneOrientation(j, orientation);
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

		if(animVersion >= 2) {
			auto numScales = f.Read<uint32_t>();
			auto &boneScales = frame->GetBoneScales();
			boneScales.resize(numScales);
			f.Read(boneScales.data(), boneScales.size() * sizeof(boneScales.front()));
		}

		unsigned short numEvents = f.Read<unsigned short>();
		for(unsigned short j = 0; j < numEvents; j++) {
			pragma::AnimationEvent *ev = new pragma::AnimationEvent;
			if(mdlVersion >= 0x0013) {
				auto name = f.ReadString();
				auto id = pragma::animation::Animation::GetEventEnumRegister().RegisterEnum(name);
				ev->eventID = (id != pragma::util::INVALID_ENUM) ? static_cast<pragma::AnimationEvent::Type>(id) : pragma::AnimationEvent::Type::Invalid;
			}
			else
				ev->eventID = static_cast<pragma::AnimationEvent::Type>(f.Read<unsigned short>());
			unsigned char numParams = f.Read<unsigned char>();
			for(unsigned char k = 0; k < numParams; k++) {
				std::string param = f.ReadString();
				ev->arguments.push_back(param);
			}
			anim->AddEvent(i, ev);
		}
		if(bHasMovement == true) {
			Vector2 move(0, 0);
			if(bMoveX == true)
				move.x = f.Read<float>();
			if(bMoveZ == true)
				move.y = f.Read<float>();
			if(mdlVersion < 0x0009)
				move *= static_cast<float>(fps);
			frame->SetMoveOffset(move);
		}
		anim->AddFrame(frame);
	}
	//fclose(m_file);
	return anim;
}

std::shared_ptr<pragma::animation::Animation> FWAD::Load(unsigned short version, const char *animation)
{
	std::string pathCache(animation);
	pragma::string::to_lower(pathCache);

	std::string path = "models\\";
	path += animation;
	const char *cPath = path.c_str();
	auto f = pragma::fs::open_file(cPath, pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary);
	if(f == nullptr) {
		Con::COUT << "WARNING: Unable to open animation '" << animation << "': File not found!" << Con::endl;
		return nullptr;
	}
	pragma::fs::File fp {f};
	auto anim = ReadData(version, fp);
	return anim;
}
