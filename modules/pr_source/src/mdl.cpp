#include "mdl.h"
#include "mdl_sequence.h"
#include "mdl_attachment.h"
#include "mdl_animation.h"
#include "mdl_bone.h"
#include "mdl_hitboxset.h"
#include "vtx.h"
#include "vvd.h"
#include "phy.h"
#include "ani.h"
#include "wv_source.hpp"
#include <pragma/networkstate/networkstate.h>
#include <pragma/model/modelmesh.h>
#include <pragma/model/animation/activities.h>
#include <fsys/filesystem.h>
#include <pragma/model/model.h>
#include <stack>
#include <pragma/physics/collisionmesh.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_string.h>
#include <pragma/model/animation/vertex_animation.hpp>
#include <pragma/game/game_limits.h>

#pragma optimize("",off)
static const std::unordered_map<std::string,Activity> translateActivities = {
	{"ACT_RESET",Activity::Invalid},
	{"ACT_IDLE",Activity::Idle},
	{"ACT_TRANSITION",Activity::Invalid},
	{"ACT_COVER",Activity::Invalid},
	{"ACT_COVER_MED",Activity::Invalid},
	{"ACT_COVER_LOW",Activity::Invalid},
	{"ACT_WALK",Activity::Walk},
	{"ACT_WALK_AIM",Activity::Invalid},
	{"ACT_WALK_CROUCH",Activity::Invalid},
	{"ACT_WALK_CROUCH_AIM",Activity::Invalid},
	{"ACT_RUN",Activity::Run},
	{"ACT_RUN_AIM",Activity::Invalid},
	{"ACT_RUN_CROUCH",Activity::Invalid},
	{"ACT_RUN_CROUCH_AIM",Activity::Invalid},
	{"ACT_RUN_PROTECTED",Activity::Invalid},
	{"ACT_SCRIPT_CUSTOM_MOVE",Activity::Invalid},
	{"ACT_RANGE_ATTACK1",Activity::RangeAttack1},
	{"ACT_RANGE_ATTACK2",Activity::RangeAttack2},
	{"ACT_RANGE_ATTACK1_LOW",Activity::RangeAttack3},
	{"ACT_RANGE_ATTACK2_LOW",Activity::RangeAttack4},
	{"ACT_DIESIMPLE",Activity::Invalid},
	{"ACT_DIEBACKWARD",Activity::Invalid},
	{"ACT_DIEFORWARD",Activity::Invalid},
	{"ACT_DIEVIOLENT",Activity::Invalid},
	{"ACT_DIERAGDOLL",Activity::Invalid},
	{"ACT_FLY",Activity::Invalid},
	{"ACT_HOVER",Activity::Invalid},
	{"ACT_GLIDE",Activity::Invalid},
	{"ACT_SWIM",Activity::Invalid},
	{"ACT_JUMP",Activity::Jump},
	{"ACT_HOP",Activity::Invalid},
	{"ACT_LEAP",Activity::Invalid},
	{"ACT_LAND",Activity::Invalid},
	{"ACT_CLIMB_UP",Activity::Invalid},
	{"ACT_CLIMB_DOWN",Activity::Invalid},
	{"ACT_CLIMB_DISMOUNT",Activity::Invalid},
	{"ACT_SHIPLADDER_UP",Activity::Invalid},
	{"ACT_SHIPLADDER_DOWN",Activity::Invalid},
	{"ACT_STRAFE_LEFT",Activity::Invalid},
	{"ACT_STRAFE_RIGHT",Activity::Invalid},
	{"ACT_ROLL_LEFT",Activity::Invalid},
	{"ACT_ROLL_RIGHT",Activity::Invalid},
	{"ACT_TURN_LEFT",Activity::TurnLeft},
	{"ACT_TURN_RIGHT",Activity::TurnRight},
	{"ACT_CROUCH",Activity::Crouch},
	{"ACT_CROUCHIDLE",Activity::CrouchIdle},
	{"ACT_STAND",Activity::Invalid},
	{"ACT_USE",Activity::Invalid},
	{"ACT_SIGNAL1",Activity::Invalid},
	{"ACT_SIGNAL2",Activity::Invalid},
	{"ACT_SIGNAL3",Activity::Invalid},
	{"ACT_SIGNAL_ADVANCE",Activity::Invalid},
	{"ACT_SIGNAL_FORWARD",Activity::Invalid},
	{"ACT_SIGNAL_GROUP",Activity::Invalid},
	{"ACT_SIGNAL_HALT",Activity::Invalid},
	{"ACT_SIGNAL_LEFT",Activity::Invalid},
	{"ACT_SIGNAL_RIGHT",Activity::Invalid},
	{"ACT_SIGNAL_TAKECOVER",Activity::Invalid},
	{"ACT_LOOKBACK_RIGHT",Activity::Invalid},
	{"ACT_LOOKBACK_LEFT",Activity::Invalid},
	{"ACT_COWER",Activity::Invalid},
	{"ACT_SMALL_FLINCH",Activity::Invalid},
	{"ACT_BIG_FLINCH",Activity::Invalid},
	{"ACT_MELEE_ATTACK1",Activity::MeleeAttack1},
	{"ACT_MELEE_ATTACK2",Activity::MeleeAttack2},
	{"ACT_RELOAD",Activity::Invalid},
	{"ACT_RELOAD_LOW",Activity::Invalid},
	{"ACT_ARM",Activity::Invalid},
	{"ACT_DISARM",Activity::Invalid},
	{"ACT_PICKUP_GROUND",Activity::Invalid},
	{"ACT_PICKUP_RACK",Activity::Invalid},
	{"ACT_IDLE_ANGRY",Activity::Invalid},
	{"ACT_IDLE_RELAXED",Activity::Invalid},
	{"ACT_IDLE_STIMULATED",Activity::Invalid},
	{"ACT_IDLE_AGITATED",Activity::Invalid},
	{"ACT_WALK_RELAXED",Activity::Invalid},
	{"ACT_WALK_STIMULATED",Activity::Invalid},
	{"ACT_WALK_AGITATED",Activity::Invalid},
	{"ACT_RUN_RELAXED",Activity::Invalid},
	{"ACT_RUN_STIMULATED",Activity::Invalid},
	{"ACT_RUN_AGITATED",Activity::Invalid},
	{"ACT_IDLE_AIM_RELAXED",Activity::Invalid},
	{"ACT_IDLE_AIM_STIMULATED",Activity::Invalid},
	{"ACT_IDLE_AIM_AGITATED",Activity::Invalid},
	{"ACT_WALK_AIM_RELAXED",Activity::Invalid},
	{"ACT_WALK_AIM_STIMULATED",Activity::Invalid},
	{"ACT_WALK_AIM_AGITATED",Activity::Invalid},
	{"ACT_RUN_AIM_RELAXED",Activity::Invalid},
	{"ACT_RUN_AIM_STIMULATED",Activity::Invalid},
	{"ACT_RUN_AIM_AGITATED",Activity::Invalid},
	{"ACT_WALK_HURT",Activity::Invalid},
	{"ACT_RUN_HURT",Activity::Invalid},
	{"ACT_SPECIAL_ATTACK1",Activity::Invalid},
	{"ACT_SPECIAL_ATTACK2",Activity::Invalid},
	{"ACT_COMBAT_IDLE",Activity::Invalid},
	{"ACT_WALK_SCARED",Activity::Invalid},
	{"ACT_RUN_SCARED",Activity::Invalid},
	{"ACT_VICTORY_DANCE",Activity::Invalid},
	{"ACT_DIE_HEADSHOT",Activity::Invalid},
	{"ACT_DIE_CHESTSHOT",Activity::Invalid},
	{"ACT_DIE_GUTSHOT",Activity::Invalid},
	{"ACT_DIE_BACKSHOT",Activity::Invalid},
	{"ACT_FLINCH_HEAD",Activity::FlinchHead},
	{"ACT_FLINCH_CHEST",Activity::FlinchChest},
	{"ACT_FLINCH_STOMACH",Activity::FlinchStomach},
	{"ACT_FLINCH_LEFTARM",Activity::FlinchLeftArm},
	{"ACT_FLINCH_RIGHTARM",Activity::FlinchRightArm},
	{"ACT_FLINCH_LEFTLEG",Activity::FlinchLeftLeg},
	{"ACT_FLINCH_RIGHTLEG",Activity::FlinchRightLeg},
	{"ACT_FLINCH_PHYSICS",Activity::FlinchGeneric},
	{"ACT_IDLE_ON_FIRE",Activity::Invalid},
	{"ACT_WALK_ON_FIRE",Activity::Invalid},
	{"ACT_RUN_ON_FIRE",Activity::Invalid},
	{"ACT_RAPPEL_LOOP",Activity::Invalid},
	{"ACT_180_LEFT",Activity::Invalid},
	{"ACT_180_RIGHT",Activity::Invalid},
	{"ACT_90_LEFT",Activity::Invalid},
	{"ACT_90_RIGHT",Activity::Invalid},
	{"ACT_STEP_LEFT",Activity::Invalid},
	{"ACT_STEP_RIGHT",Activity::Invalid},
	{"ACT_STEP_BACK",Activity::Invalid},
	{"ACT_STEP_FORE",Activity::Invalid},
	{"ACT_GESTURE_RANGE_ATTACK1",Activity::Invalid},
	{"ACT_GESTURE_RANGE_ATTACK2",Activity::Invalid},
	{"ACT_GESTURE_MELEE_ATTACK1",Activity::Invalid},
	{"ACT_GESTURE_MELEE_ATTACK2",Activity::Invalid},
	{"ACT_GESTURE_RANGE_ATTACK1_LOW",Activity::Invalid},
	{"ACT_GESTURE_RANGE_ATTACK2_LOW",Activity::Invalid},
	{"ACT_MELEE_ATTACK_SWING_GESTURE",Activity::Invalid},
	{"ACT_GESTURE_SMALL_FLINCH",Activity::Invalid},
	{"ACT_GESTURE_BIG_FLINCH",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_BLAST",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_HEAD",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_CHEST",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_STOMACH",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_LEFTARM",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_RIGHTARM",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_LEFTLEG",Activity::Invalid},
	{"ACT_GESTURE_FLINCH_RIGHTLEG",Activity::Invalid},
	{"ACT_GESTURE_TURN_LEFT",Activity::Invalid},
	{"ACT_GESTURE_TURN_RIGHT",Activity::Invalid},
	{"ACT_GESTURE_TURN_LEFT45",Activity::Invalid},
	{"ACT_GESTURE_TURN_RIGHT45",Activity::Invalid},
	{"ACT_GESTURE_TURN_LEFT90",Activity::Invalid},
	{"ACT_GESTURE_TURN_RIGHT90",Activity::Invalid},
	{"ACT_GESTURE_TURN_LEFT45_FLAT",Activity::Invalid},
	{"ACT_GESTURE_TURN_RIGHT45_FLAT",Activity::Invalid},
	{"ACT_GESTURE_TURN_LEFT90_FLAT",Activity::Invalid},
	{"ACT_GESTURE_TURN_RIGHT90_FLAT",Activity::Invalid},

	{"ACT_VM_IDLE",Activity::VmIdle},
	{"ACT_VM_DRAW",Activity::VmDeploy},
	{"ACT_VM_HOLSTER",Activity::VmHolster},
	{"ACT_VM_PRIMARYATTACK",Activity::VmPrimaryAttack},
	{"ACT_VM_SECONDARYATTACK",Activity::VmSecondaryAttack},
	{"ACT_VM_RELOAD",Activity::VmReload},

	{"ACT_GESTURE_TURN_LEFT",Activity::GestureTurnLeft},
	{"ACT_GESTURE_TURN_RIGHT",Activity::GestureTurnRight}
};

static const std::unordered_map<int32_t,AnimationEvent::Type> translateAnimEvent = {
	{3,AnimationEvent::Type::EmitSound}
};

Vector3 import::mdl::transform_coordinate_system(const Vector3 &v)
{
	auto r = v;
	auto y = r.y;
	r.y = r.z;
	r.z = -y;
	return r;
}

void transform_local_frame_to_global(const std::vector<std::shared_ptr<import::mdl::Bone>> &bones,Frame &frame)
{
	std::function<void(const std::vector<std::shared_ptr<import::mdl::Bone>>&,const Vector3&,const Quat&)> transform_local_transforms_to_global = nullptr;
	transform_local_transforms_to_global = [&transform_local_transforms_to_global,&frame](const std::vector<std::shared_ptr<import::mdl::Bone>> &bones,const Vector3 &origin,const Quat &originRot) {
		for(auto &bone : bones)
		{
			auto boneId = bone->GetID();
			auto pos = *frame.GetBonePosition(boneId);
			uvec::rotate(&pos,originRot);
			pos += origin;
			frame.SetBonePosition(boneId,pos);

			auto rot = *frame.GetBoneOrientation(boneId);
			rot = originRot *rot;
			frame.SetBoneOrientation(boneId,rot);

			transform_local_transforms_to_global(bone->GetChildren(),pos,rot);
		}
	};
	transform_local_transforms_to_global(bones,{},{});
}

bool import::load_mdl(NetworkState *nw,const VFilePtr &f,const std::function<std::shared_ptr<Model>()> &fCreateModel,const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)> &fCallback,bool bCollision,MdlInfo &mdlInfo)
{
	auto offset = f->Tell();
	auto &header = mdlInfo.header = f->Read<mdl::studiohdr_t>();
	auto *id = reinterpret_cast<uint8_t*>(&header.id);
	if(id[0] != 0x49 || id[1] != 0x44 || id[2] != 0x53 || id[3] != 0x54)
		return false;

	auto eyePos = header.eyeposition;
	umath::swap(eyePos.y,eyePos.z);
	umath::negate(eyePos.z);
	mdlInfo.model.SetEyeOffset(eyePos);

	// Read includes
	f->Seek(offset +header.includemodelindex);
	for(auto i=decltype(header.numincludemodels){0};i<header.numincludemodels;++i)
	{
		auto offset = f->Tell();
		auto labelOffset = f->Read<int32_t>();
		auto fileNameOffset = f->Read<int32_t>();
		if(labelOffset != 0)
		{
			f->Seek(offset +labelOffset);
			auto label = f->ReadString();
		}
		if(fileNameOffset != 0)
		{
			f->Seek(offset +fileNameOffset);
			auto fileName = f->ReadString();
			if(fileName.empty() == false)
			{
				auto mdlName = FileManager::GetCanonicalizedPath(fileName);
				if(ustring::substr(mdlName,0,7) == std::string("models") +FileManager::GetDirectorySeparator())
					mdlName = ustring::substr(mdlName,7);
				ufile::remove_extension_from_filename(mdlName);
				auto wmdPath = mdlName +".wmd";
				mdlInfo.model.GetMetaInfo().includes.push_back(wmdPath);
				if(FileManager::Exists("models\\" +wmdPath) == false)
				{
					auto r = convert_hl2_model(nw,fCreateModel,fCallback,"models\\",mdlName);
					if(r == false)
						std::cout<<"WARNING: Unable to convert model '"<<fileName<<"'!"<<std::endl;
				}
			}
		}
	}

	// Read textures
	f->Seek(offset +header.textureindex);
	mdlInfo.textures.reserve(header.numtextures);
	mdlInfo.texturePaths.reserve(header.numtextures);
	auto fAddPath = [&mdlInfo](const std::string &fpath) {
		auto npath = FileManager::GetCanonicalizedPath(fpath);
		if(npath.empty())
			return;
		auto it = std::find_if(mdlInfo.texturePaths.begin(),mdlInfo.texturePaths.end(),[&npath](const std::string &pathOther) {
			return ustring::compare(npath,pathOther,false);
		});
		if(it != mdlInfo.texturePaths.end())
			return;
		mdlInfo.texturePaths.push_back(npath);
	};
	for(auto i=decltype(header.numtextures){0};i<header.numtextures;++i)
	{
		auto stdTexOffset = f->Tell();
		auto stdTex = f->Read<mdl::mstudiotexture_t>();
		auto postOffset = f->Tell();

		f->Seek(stdTexOffset +stdTex.sznameindex);
		auto name = f->ReadString();

		if(name.empty() == false)
		{
			mdlInfo.textures.push_back(ufile::get_file_from_filename(name));
			fAddPath(ufile::get_path_from_filename(name));
		}

		f->Seek(postOffset);
	}
	//

	// Read texture paths
	f->Seek(offset +header.cdtextureindex);
	mdlInfo.texturePaths.reserve(mdlInfo.texturePaths.size() +header.numcdtextures);
	for(auto i=decltype(header.numcdtextures){0};i<header.numcdtextures;++i)
	{
		f->Seek(offset +header.cdtextureindex +i *sizeof(int32_t));
		auto strOffset = f->Read<int32_t>();
		f->Seek(offset +strOffset);
		auto stdCdTex = f->ReadString();
		fAddPath(stdCdTex);
	}
	//

	// Read bones
	f->Seek(offset +header.boneindex);
	auto &bones = mdlInfo.bones;
	bones.reserve(header.numbones);
	for(auto i=decltype(header.numbones){0};i<header.numbones;++i)
		bones.push_back(std::make_shared<mdl::Bone>(i,f));
	if(header.numbones > umath::to_integral(GameLimits::MaxBones))
		std::cout<<"WARNING: Model has "<<header.numbones<<" bones, but the engine only supports up to "<<umath::to_integral(GameLimits::MaxBones)<<"! Expect issues!"<<std::endl;
	mdl::Bone::BuildHierarchy(bones);

	// Read animation names
	//f->Seek(offset +header.szanimblocknameindex); // 0?
	//auto name = f->ReadString();
	//std::cout<<"Anim Block Name: "<<name<<std::endl;
	//
	
	// Read animations
	std::vector<std::string> aniFileNames;
	auto &animBlocks = mdlInfo.animationBlocks;
	if(header.numanimblocks > 0)
	{
		animBlocks.reserve(header.numanimblocks);
		if(header.szanimblocknameindex > 0)
		{
			auto offset = f->Tell();
			f->Seek(header.szanimblocknameindex);
			auto animBlockName = f->ReadString();
			aniFileNames.push_back(animBlockName);

			f->Seek(offset);
		}
		if(header.animblockindex > 0)
		{
			f->Seek(offset +header.animblockindex);
			for(auto i=decltype(header.numanimblocks){0};i<header.numanimblocks;++i)
				animBlocks.push_back(f->Read<mdl::mstudioanimblock_t>());
		}
	}

	f->Seek(offset +header.localanimindex);

	auto &animDescs = mdlInfo.animationDescs;
	animDescs.reserve(header.numlocalanim);
	auto animDescOffset = f->Tell();
	for(auto i=decltype(header.numlocalanim){0};i<header.numlocalanim;++i)
		animDescs.push_back(mdl::AnimationDesc(i,f));

	// Read animation sections
	for(auto i=decltype(header.numlocalanim){0};i<header.numlocalanim;++i)
	{
		auto &animDesc = animDescs[i];
		animDesc.ReadAnimationSections(header,animDescOffset +i *sizeof(mdl::mstudioanimdesc_t),f);
	}
	//

	for(auto it=animDescs.begin();it!=animDescs.end();++it)
	{
		auto &animDesc = *it;
		animDesc.ReadAnimation(header,animDescOffset,animDescOffset +(it -animDescs.begin()) *sizeof(mdl::mstudioanimdesc_t),f);
	}
	//

	// Load sequences
	f->Seek(offset +header.localseqindex);
	auto &sequences = mdlInfo.sequences;
	sequences.reserve(header.numlocalseq);
	for(auto i=decltype(header.numlocalseq){0};i<header.numlocalseq;++i)
		sequences.push_back({f,header.numbones});
	//

	// Read attachments
	f->Seek(offset +header.localattachmentindex);
	auto &attachments = mdlInfo.attachments;
	attachments.reserve(header.numlocalattachments);
	for(auto i=decltype(header.numlocalattachments){0};i<header.numlocalattachments;++i)
		attachments.push_back(std::make_shared<mdl::Attachment>(f));
	//

	// Read flex descs
	f->Seek(offset +header.flexdescindex);
	auto &flexDescs = mdlInfo.flexDescs;
	flexDescs.reserve(header.numflexdesc);
	for(auto i=decltype(header.numflexdesc){0};i<header.numflexdesc;++i)
		flexDescs.push_back(mdl::FlexDesc{f});
	//

	// Read flex controllers
	f->Seek(offset +header.flexcontrollerindex);
	auto &flexControllers = mdlInfo.flexControllers;
	flexControllers.reserve(header.numflexcontrollers);
	for(auto i=decltype(header.numflexcontrollers){0};i<header.numflexcontrollers;++i)
		flexControllers.push_back(mdl::FlexController{f});
	//

	// Read flex rules
	f->Seek(offset +header.flexruleindex);
	auto &flexRules = mdlInfo.flexRules;
	flexRules.reserve(header.numflexrules);
	for(auto i=decltype(header.numflexrules){0};i<header.numflexrules;++i)
		flexRules.push_back(mdl::FlexRule{f});
	//

	// Read flex controller uis
	f->Seek(offset +header.flexcontrolleruiindex);
	auto &flexControllerUis = mdlInfo.flexControllerUis;
	flexControllerUis.reserve(header.numflexcontrollerui);
	for(auto i=decltype(header.numflexcontrollerui){0};i<header.numflexcontrollerui;++i)
		flexControllerUis.push_back(mdl::FlexControllerUi{f});
	//

	if(flexRules.empty() == false)
	{
		for(auto &flexRule : flexRules)
		{
			auto &flexDesc = flexDescs.at(flexRule.GetFlexId());
			auto &ops = flexRule.GetOperations();
			auto flexName = flexDesc.GetName();
			std::stringstream sop;
			sop<<"%"<<flexName<<" = ";

			struct OpExpression
			{
				OpExpression(const std::string &e="",uint32_t p=0u)
					: expression(e),precedence(p)
				{}
				std::string expression;
				uint32_t precedence = 0u;
			};

			std::stack<OpExpression> opStack {};

			auto fCheckStackCount = [&opStack,&flexName](uint32_t required,const std::string &identifier) -> bool {
				if(opStack.size() >= required)
					return true;
				std::cout<<"WARNING: Unable to evaluate flex operation '"<<identifier<<"' for flex "<<flexName<<"! Skipping..."<<std::endl;
				opStack.push({"0.0"});
				return false;
			};

			for(auto &op : ops)
			{
				switch(op.type)
				{
					case mdl::FlexRule::Operation::Type::None:
						break;
					case mdl::FlexRule::Operation::Type::Const:
						opStack.push({std::to_string(op.d.value),10u});
						break;
					case mdl::FlexRule::Operation::Type::Fetch:
						opStack.push({flexControllers.at(op.d.index).GetName(),10u});
						break;
					case mdl::FlexRule::Operation::Type::Fetch2:
						opStack.push({"%" +flexDescs.at(op.d.index).GetName(),10u});
						break;
					case mdl::FlexRule::Operation::Type::Add:
					{
						if(fCheckStackCount(2,"add") == false)
							break;
						auto r = opStack.top();
						opStack.pop();
						auto l = opStack.top();
						opStack.pop();

						opStack.push({l.expression +" + " +r.expression,1u});
						break;
					}
					case mdl::FlexRule::Operation::Type::Sub:
					{
						if(fCheckStackCount(2,"sub") == false)
							break;
						auto r = opStack.top();
						opStack.pop();
						auto l = opStack.top();
						opStack.pop();

						opStack.push({l.expression +" - " +r.expression,1u});
						break;
					}
					case mdl::FlexRule::Operation::Type::Mul:
					{
						if(fCheckStackCount(2,"mul") == false)
							break;
						auto r = opStack.top();
						opStack.pop();
						auto rExpr = std::string{};
						if(r.precedence < 2)
							rExpr = "(" +r.expression +")";
						else
							rExpr = r.expression;

						auto l = opStack.top();
						opStack.pop();
						auto lExpr = std::string{};
						if(l.precedence < 2)
							lExpr = "(" +l.expression +")";
						else
							lExpr = l.expression;

						opStack.push({lExpr +" * "+rExpr,2});
						break;
					}
					case mdl::FlexRule::Operation::Type::Div:
					{
						if(fCheckStackCount(2,"div") == false)
							break;
						auto r = opStack.top();
						opStack.pop();
						auto rExpr = std::string{};
						if(r.precedence < 2)
							rExpr = "(" +r.expression +")";
						else
							rExpr = r.expression;

						auto l = opStack.top();
						opStack.pop();
						auto lExpr = std::string{};
						if(l.precedence < 2)
							lExpr = "(" +l.expression +")";
						else
							lExpr = l.expression;

						opStack.push({lExpr +" / "+rExpr,2});
						break;
					}
					case mdl::FlexRule::Operation::Type::Neg:
					{
						if(fCheckStackCount(1,"neg") == false)
							break;
						auto r = opStack.top();
						opStack.pop();
						opStack.push({"-" +r.expression,10});
						break;
					}
					case mdl::FlexRule::Operation::Type::Exp:
						std::cout<<"WARNING: Invalid flex rule "<<umath::to_integral(op.type)<<" (Exp)! Skipping..."<<std::endl;
						break;
					case mdl::FlexRule::Operation::Type::Open:
						std::cout<<"WARNING: Invalid flex rule "<<umath::to_integral(op.type)<<" (Open)! Skipping..."<<std::endl;
						break;
					case mdl::FlexRule::Operation::Type::Close:
						std::cout<<"WARNING: Invalid flex rule "<<umath::to_integral(op.type)<<" (Close)! Skipping..."<<std::endl;
						break;
					case mdl::FlexRule::Operation::Type::Comma:
						std::cout<<"WARNING: Invalid flex rule "<<umath::to_integral(op.type)<<" (Comma)! Skipping..."<<std::endl;
						break;
					case mdl::FlexRule::Operation::Type::Max:
					{
						if(fCheckStackCount(2,"max") == false)
							break;
						auto r = opStack.top();
						opStack.pop();
						auto rExpr = std::string{};
						if(r.precedence < 5)
							rExpr = "(" +r.expression +")";
						else
							rExpr = r.expression;

						auto l = opStack.top();
						opStack.pop();
						auto lExpr = std::string{};
						if(l.precedence < 5)
							lExpr = "(" +l.expression +")";
						else
							lExpr = l.expression;

						opStack.push({" max(" +lExpr +", "+rExpr +")",5});
						break;
					}
					case mdl::FlexRule::Operation::Type::Min:
					{
						if(fCheckStackCount(2,"min") == false)
							break;
						auto r = opStack.top();
						opStack.pop();
						auto rExpr = std::string{};
						if(r.precedence < 5)
							rExpr = "(" +r.expression +")";
						else
							rExpr = r.expression;

						auto l = opStack.top();
						opStack.pop();
						auto lExpr = std::string{};
						if(l.precedence < 5)
							lExpr = "(" +l.expression +")";
						else
							lExpr = l.expression;

						opStack.push({" min(" +lExpr +", "+rExpr +")",5});
						break;
					}
					case mdl::FlexRule::Operation::Type::TwoWay0:
						opStack.push({" (1 - (min(max(" +flexControllers.at(op.d.index).GetName() + " + 1, 0), 1)))",5});
						break;
					case mdl::FlexRule::Operation::Type::TwoWay1:
						opStack.push({" (min(max(" +flexControllers.at(op.d.index).GetName() + ", 0), 1))",5});
						break;
					case mdl::FlexRule::Operation::Type::NWay:
					{
						auto &v = flexControllers.at(op.d.index).GetName();

						auto &valueControllerIndex = opStack.top();
						opStack.pop();
						auto &flValue = flexControllers.at(ustring::to_int(valueControllerIndex.expression)).GetName();

						auto filterRampW = opStack.top();
						opStack.pop();
						auto filterRampZ = opStack.top();
						opStack.pop();
						auto filterRampY = opStack.top();
						opStack.pop();
						auto filterRampX = opStack.top();
						opStack.pop();

						auto greaterThanX = "min(1, (-min(0, (" + filterRampX.expression + " - " + flValue + "))))";
						auto lessThanY = "min(1, (-min(0, (" + flValue + " - " + filterRampY.expression + "))))";
						auto remapX = "min(max((" + flValue + " - " + filterRampX.expression + ") / (" + filterRampY.expression + " - " + filterRampX.expression + "), 0), 1)";
						auto greaterThanEqualY = "-(min(1, (-min(0, (" + flValue + " - " + filterRampY.expression + ")))) - 1)";
						auto lessThanEqualZ = "-(min(1, (-min(0, (" + filterRampZ.expression + " - " + flValue + ")))) - 1)";
						auto greaterThanZ = "min(1, (-min(0, (" + filterRampZ.expression + " - " + flValue + "))))";
						auto lessThanW = "min(1, (-min(0, (" + flValue + " - " + filterRampW.expression + "))))";
						auto remapZ = "(1 - (min(max((" + flValue + " - " + filterRampZ.expression + ") / (" + filterRampW.expression + " - " + filterRampZ.expression + "), 0), 1)))";

						auto expValue = "((" + greaterThanX + " * " + lessThanY + ") * " + remapX + ") + (" + greaterThanEqualY + " * " + lessThanEqualZ + ") + ((" + greaterThanZ + " * " + lessThanW + ") * " + remapZ + ")";

						opStack.push({"((" + expValue + ") * (" + v + "))",5});
						break;
					}
					case mdl::FlexRule::Operation::Type::Combo:
					{
						if(opStack.empty())
							break;
						auto count = op.d.index;
						if(fCheckStackCount(count,"combo") == false)
							break;
						auto newExpr = OpExpression{};
						auto intExpr = opStack.top();
						opStack.pop();
						newExpr.expression += intExpr.expression;
						for(auto i=decltype(count){1};i<count;++i)
						{
							intExpr = opStack.top();
							opStack.pop();
							newExpr.expression += " * " +intExpr.expression;
						}
						newExpr.expression = "(" +newExpr.expression +")";
						newExpr.precedence = 5u;
						opStack.push(newExpr);
						break;
					}
					case mdl::FlexRule::Operation::Type::Dominate:
					{
						if(opStack.empty())
							break;
						auto count = op.d.index;
						if(fCheckStackCount(count +1,"dominate") == false)
							break;
						auto newExpr = OpExpression{};
						auto intExpr = opStack.top();
						opStack.pop();
						newExpr.expression += intExpr.expression;
						for(auto i=decltype(count){1};i<count;++i)
						{
							intExpr = opStack.top();
							opStack.pop();
							newExpr.expression += " * " +intExpr.expression;
						}
						intExpr = opStack.top();
						opStack.pop();
						newExpr.expression = intExpr.expression +" * (1 - " +newExpr.expression +")";
						newExpr.expression = "(" +newExpr.expression +")";
						newExpr.precedence = 5u;
						opStack.push(newExpr);
						break;
					}
					case mdl::FlexRule::Operation::Type::DMELowerEyelid:
					{
						auto &pCloseLidV = flexControllers.at(op.d.index);
						auto range = pCloseLidV.GetRange();
						auto flCloseLidVMin = std::to_string(range.first);
						auto flCloseLidVMax = std::to_string(range.second);
						auto flCloseLidV = "(min(max((" + pCloseLidV.GetName() + " - " + flCloseLidVMin + ") / (" + flCloseLidVMax + " - " + flCloseLidVMin + "), 0), 1))";

						auto closeLidIndex = opStack.top();
						opStack.pop();

						auto &pCloseLid = flexControllers.at(ustring::to_int(closeLidIndex.expression));
						range = pCloseLid.GetRange();
						auto flCloseLidMin = std::to_string(range.first);
						auto flCloseLidMax = std::to_string(range.second);
						auto flCloseLid = "(min(max((" + pCloseLid.GetName() + " - " + flCloseLidMin + ") / (" + flCloseLidMax + " - " + flCloseLidMin + "), 0), 1))";

						opStack.pop();

						auto eyeUpDownIndex = opStack.top();
						opStack.pop();
						auto &pEyeUpDown = flexControllers.at(ustring::to_int(eyeUpDownIndex.expression));
						range = pCloseLid.GetRange();
						auto flEyeUpDownMin = std::to_string(range.first);
						auto flEyeUpDownMax = std::to_string(range.second);
						auto flEyeUpDown = "(-1 + 2 * (min(max((" + pEyeUpDown.GetName() + " - " + flEyeUpDownMin + ") / (" + flEyeUpDownMax + " - " + flEyeUpDownMin + "), 0), 1)))";

						opStack.push({"(min(1, (1 - " + flEyeUpDown + ")) * (1 - " + flCloseLidV + ") * " + flCloseLid + ")",5});
						break;
					}
					case mdl::FlexRule::Operation::Type::DMEUpperEyelid:
					{
						auto &pCloseLidV = flexControllers.at(op.d.index);
						auto range = pCloseLidV.GetRange();
						auto flCloseLidVMin = std::to_string(range.first);
						auto flCloseLidVMax = std::to_string(range.second);
						auto flCloseLidV = "(min(max((" + pCloseLidV.GetName() + " - " + flCloseLidVMin + ") / (" + flCloseLidVMax + " - " + flCloseLidVMin + "), 0), 1))";

						auto closeLidIndex = opStack.top();
						opStack.pop();
						auto &pCloseLid = flexControllers.at(ustring::to_int(closeLidIndex.expression));
						range = pCloseLidV.GetRange();
						auto flCloseLidMin = std::to_string(range.first);
						auto flCloseLidMax = std::to_string(range.second);
						auto flCloseLid = "(min(max((" + pCloseLid.GetName() + " - " + flCloseLidMin + ") / (" + flCloseLidMax + " - " + flCloseLidMin + "), 0), 1))";

						opStack.pop();

						auto eyeUpDownIndex = opStack.top();
						opStack.pop();
						auto &pEyeUpDown = flexControllers.at(ustring::to_int(eyeUpDownIndex.expression));
						range = pCloseLidV.GetRange();
						auto flEyeUpDownMin = std::to_string(range.first);
						auto flEyeUpDownMax = std::to_string(range.second);
						auto flEyeUpDown = "(-1 + 2 * (min(max((" + pEyeUpDown.GetName() + " - " + flEyeUpDownMin + ") / (" + flEyeUpDownMax + " - " + flEyeUpDownMin + "), 0), 1)))";

						opStack.push({"(min(1, (1 + " + flEyeUpDown + ")) * " + flCloseLidV + " * " + flCloseLid + ")",5});
						break;
					}
				}
			}
			if(opStack.size() == 1)
			{
				sop<<opStack.top().expression;
				//std::cout<<"Final expression: "<<sop.str()<<std::endl;
			}
			else
				std::cout<<"WARNING: Unable to evaluate flex rule for flex "<<flexName<<"! Skipping..."<<std::endl;
		}
	}

	// Read hitboxes
	f->Seek(offset +header.hitboxsetindex);
	auto &hitboxSets = mdlInfo.hitboxSets;
	hitboxSets.reserve(header.numhitboxsets);
	for(auto i=decltype(header.numhitboxsets){0};i<header.numhitboxsets;++i)
		hitboxSets.push_back({f});
	//

	// Read skins
	f->Seek(offset +header.skinindex);
	auto &skinFamilies = mdlInfo.skinFamilies;
	skinFamilies.reserve(header.numskinfamilies);
	for(auto i=decltype(header.numskinfamilies){0};i<header.numskinfamilies;++i)
	{
		skinFamilies.push_back({});
		auto &skinFamily = skinFamilies.back();
		skinFamily.resize(header.numskinref);
		f->Read(skinFamily.data(),header.numskinref *sizeof(skinFamily.front()));
	}
	//

	// Read body parts
	f->Seek(offset +header.bodypartindex);
	auto &bodyParts = mdlInfo.bodyParts;
	bodyParts.reserve(header.numbodyparts);
	for(auto i=decltype(header.numbodyparts){0};i<header.numbodyparts;++i)
		bodyParts.push_back({f});
	//

	// Read pose parameters
	f->Seek(offset +header.localposeparamindex);
	auto &poseParameters = mdlInfo.poseParameters;
	poseParameters.reserve(header.numlocalposeparameters);
	for(auto i=decltype(header.numlocalposeparameters){0};i<header.numlocalposeparameters;++i)
		poseParameters.push_back({f});
	//

	return true;
}

std::shared_ptr<Model> import::load_mdl(NetworkState *nw,const std::unordered_map<std::string,VFilePtr> &files,const std::function<std::shared_ptr<Model>()> &fCreateModel,const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)> &fCallback,bool bCollision,std::vector<std::string> &textures)
{
	auto it = files.find("mdl");
	if(it == files.end())
		return nullptr;
	auto ptrMdl = fCreateModel();
	auto &mdl = *ptrMdl;
	MdlInfo mdlInfo(mdl);
	auto r = load_mdl(nw,it->second,fCreateModel,fCallback,bCollision,mdlInfo);
	if(r == false)
		return nullptr;
	it = files.find("ani");
	if(it != files.end())
	{
		import::mdl::load_ani(it->second,mdlInfo);
		for(auto &animDesc : mdlInfo.animationDescs)
		{
			auto &sectionAnims = animDesc.GetSectionAnimations();
			//std::cout<<"Section Anim Count: "<<sectionAnims.size()<<std::endl;
		}
	}

	// Transform animations
	auto &anims = mdlInfo.animations;
	auto &animDescs = mdlInfo.animationDescs;
	anims.resize(animDescs.size());
	for(auto i=decltype(animDescs.size()){0};i<animDescs.size();++i)
	{
		auto &animDesc = animDescs[i];

		anims[i] = animDesc.CalcAnimation(mdlInfo);
	}
	//

	auto &header = mdlInfo.header;
	mdl.SetMass(header.mass);
	textures = mdlInfo.textures;
	for(auto &texPath : mdlInfo.texturePaths)
		mdl.AddTexturePath(texPath);
	auto &texPaths = mdl.GetTexturePaths();
	std::vector<uint32_t> textureTranslations;
	textureTranslations.reserve(textures.size());
	for(auto &tex : textures)
	{
		// Some models (e.g. tf2 models) contain the absolute texture path for some reason.
		// We'll strip it here if it's in one of our known texture paths.
		auto texPath = FileManager::GetCanonicalizedPath(tex);
		auto it = std::find_if(texPaths.begin(),texPaths.end(),[&texPath](const std::string &path) {
			return (ustring::sub(texPath,0,path.length()) == path) ? true : false;
		});
		if(it != texPaths.end())
			texPath = ustring::sub(texPath,it->length());
		textureTranslations.push_back(import::util::add_texture(*nw,mdl,texPath));
	}

	auto &skeleton = mdl.GetSkeleton();
	auto &skelBones = skeleton.GetBones();
	auto &bones = mdlInfo.bones;
	for(auto &bone : bones)
	{
		auto *mdlBone = new Bone();
		mdlBone->name = bone->GetName();
		skeleton.AddBone(mdlBone);
	}
	
	for(auto &pp : mdlInfo.poseParameters)
	{
		auto start = pp.GetStart();
		auto end = pp.GetEnd();
		auto loopPoint = pp.GetLoop();
		auto loop = (loopPoint > start && loopPoint < end) ? false : true;
		if(ustring::compare(pp.GetName(),"move_yaw",false)) // TODO: Dirty hack. Some models (e.g. gman.mdl) have no loop point for controllers that SHOULD have loop points (e.g. move_yaw). What to do about this? (Could change range of move_yaw from [0,360] to [-180,180] in-engine.)
			loop = true;
		mdl.AddBlendController(pp.GetName(),start,end,loop);
	}

	auto reference = std::make_shared<Animation>();
	mdl.AddAnimation("reference",reference); // Reference always has to be the first animation!
	auto bHasReferenceAnim = false;

	std::vector<decltype(anims.size())> animsWithSequences;
	animsWithSequences.reserve(mdlInfo.sequences.size());
	std::unordered_map<uint32_t,uint32_t> seqAnims; // Map sequence to animation
	std::vector<std::string> animNames(anims.size());
	for(auto i=decltype(mdlInfo.sequences.size()){0};i<mdlInfo.sequences.size();++i)
	{
		auto &seq = mdlInfo.sequences.at(i);
		auto &animIndices = seq.GetAnimationIndices();
		auto &name = seq.GetName();
		auto &activityName = seq.GetActivityName();
		if(animIndices.empty())
			continue;
		auto animIdx = animIndices.front();
		animNames.at(animIdx) = name;
		std::shared_ptr<Animation> anim = nullptr;
		auto it = std::find(animsWithSequences.begin(),animsWithSequences.end(),static_cast<decltype(anims.size())>(animIdx));
		if(it == animsWithSequences.end())
		{
			animsWithSequences.push_back(animIdx);
			anim = anims[animIdx]; // Use existing anim
		}
		else // There was already a sequence with this animation; Copy it
		{
			anim = anims[animIdx];
			anim = std::make_shared<Animation>(*anim);//,Animation::ShareMode::Frames);
		}
		
		auto itAct = translateActivities.find(seq.GetActivityName());
		if(itAct != translateActivities.end() && itAct->second != Activity::Invalid)
			anim->SetActivity(itAct->second);
		else
			anim->SetActivity(static_cast<Activity>(Animation::GetActivityEnumRegister().RegisterEnum(activityName)));
		anim->SetActivityWeight(seq.GetActivityWeight());
		anim->SetFadeInTime(seq.GetFadeInTime());
		anim->SetFadeOutTime(seq.GetFadeOutTime());
		anim->SetRenderBounds(seq.GetMin(),seq.GetMax());
		anim->GetBoneWeights() = seq.GetWeights();

		auto flags = seq.GetFlags();
		if(flags &STUDIO_LOOPING)
			anim->AddFlags(FAnim::Loop);
		if(flags &STUDIO_AUTOPLAY)
			anim->AddFlags(FAnim::Autoplay | FAnim::Loop); // Autoplay animations are always looped
		if(flags &STUDIO_DELTA)
			anim->AddFlags(FAnim::Gesture);

		auto numFrames = anim->GetFrameCount();
		for(auto &ev : seq.GetEvents())
		{
			auto *animEv = new AnimationEvent();
			animEv->eventID = static_cast<AnimationEvent::Type>(ev.GetEvent());
			auto itTranslate = translateAnimEvent.find(umath::to_integral(animEv->eventID));
			if(itTranslate != translateAnimEvent.end())
				animEv->eventID = itTranslate->second;

			animEv->arguments.push_back(ev.GetOptions());
			auto frame = umath::round(ev.GetCycle() *numFrames);
			anim->AddEvent(frame,animEv);
		}
		auto newAnimId = mdl.AddAnimation(name,anim);
		seqAnims.insert(std::make_pair(i,newAnimId));
		if(name == "reference")
			bHasReferenceAnim = true;
	}

	for(auto i=decltype(anims.size()){0};i<anims.size();++i)
	{
		auto &anim = anims[i];
		if(anim == nullptr)
			continue;
		auto it = std::find(animsWithSequences.begin(),animsWithSequences.end(),i);
		if(it == animsWithSequences.end()) // Only add this animation if it hasn't already been added through a sequence
		{
			auto &name = animDescs.at(i).GetName();
			mdl.AddAnimation(name,anim);
			if(name == "reference")
				bHasReferenceAnim = true;
			animNames.at(i) = name;
		}
	}

	// Blend controllers
	auto &mdlAnims = mdl.GetAnimations();
	for(auto i=decltype(mdlInfo.sequences.size()){0};i<mdlInfo.sequences.size();++i)
	{
		auto &seq = mdlInfo.sequences.at(i);
		auto &pp = seq.GetPoseParameter();
		if(pp.numBlends < 2)
			continue;
		auto it = seqAnims.find(i);
		if(it == seqAnims.end())
			continue;
		auto animIdx = it->second;
		auto &mdlAnim = mdlAnims.at(animIdx);
		auto *bc = mdlAnim->SetBlendController(pp.paramIdx.at(0));

		const auto flags = STUDIO_AUTOPLAY | STUDIO_DELTA;
		auto bAutoplayGesture = (seq.GetFlags() &flags) == flags;
		// If this is an autoplay gesture, all animations associated with this one should probably loop

		auto &animIndices = seq.GetAnimationIndices();
		/*std::cout<<"Name: "<<seq.GetName()<<std::endl;
		for(auto idx : animIndices)
		{
			if(idx == -1)
				continue;
			auto &animName = animDescs.at(idx).GetName();
			auto &seqName = animNames.at(idx);
			std::cout<<"\tChild anim "<<idx<<": "<<animName<<" ("<<seqName<<")"<<std::endl;
		}*/
		for(auto i=decltype(pp.numBlends){0};i<pp.numBlends;++i)
		{
			auto animId = animIndices.at(i);
			if(animId == -1)
				continue;
			auto &name = animNames.at(animId);
			auto animDstId = mdl.LookupAnimation(name);
			//std::cout<<"Assigning animation "<<mdl.GetAnimationName(animDstId)<<std::endl;
			if(animDstId == -1)
				continue;
			if(bAutoplayGesture == true)
				mdl.GetAnimation(animDstId)->AddFlags(FAnim::Loop);
			bc->transitions.push_back({});
			auto &t = bc->transitions.back();
			t.animation = animDstId;
			t.transition = pp.start.at(0) +(pp.end.at(0) -pp.start.at(0)) *(i /static_cast<float>(pp.numBlends -1));
		}
	}
	//

	// Flex Controllers
	auto &flexControllers = mdl.GetFlexControllers();
	flexControllers.resize(mdlInfo.flexControllers.size());
	auto flexIdx = 0u;
	for(auto &fc : mdlInfo.flexControllers)
	{
		auto &outFc = flexControllers.at(flexIdx++);
		outFc.name = fc.GetName();
		auto range = fc.GetRange();
		outFc.min = range.first;
		outFc.max = range.second;
	}
	//

	// Flexes
	auto &flexes = mdl.GetFlexes();
	flexes.reserve(mdlInfo.flexDescs.size());
	flexIdx = 0u;
	for(auto &flexDesc : mdlInfo.flexDescs)
	{
		flexes.push_back(Flex{flexDesc.GetName()});
		auto &flex = flexes.back();
		auto it = std::find_if(mdlInfo.flexRules.begin(),mdlInfo.flexRules.end(),[flexIdx](const import::mdl::FlexRule &rule) {
			return (rule.GetFlexId() == flexIdx) ? true : false;
		});
		if(it != mdlInfo.flexRules.end())
		{
			auto &rule = *it;
			auto &ops = flex.GetOperations();
			auto &ruleOps = rule.GetOperations();
			ops.reserve(ruleOps.size());
			for(auto &ruleOp : ruleOps)
			{
				ops.push_back({});
				auto &op = ops.back();
				op.type = static_cast<Flex::Operation::Type>(ruleOp.type);
				op.d.value = ruleOp.d.value;
			}
		}
		++flexIdx;
	}
	//

	auto &rootBones = skeleton.GetRootBones();
	if(header.numbones > 0)
		rootBones.clear();

	for(auto i=decltype(header.numbones){0};i<header.numbones;++i)
	{
		auto &skelBone = skelBones[i];
		auto &origBone = bones[i];

		auto &origParent = origBone->GetParent();
		if(origParent != nullptr)
		{
			auto &skelParent = skelBones[origParent->GetID()];
			skelBone->parent = skelParent;
			skelParent->children[i] = skelBone;
		}
		else
			rootBones[i] = skelBone;
	}

	auto refId = mdl.LookupAnimation("reference");
	std::shared_ptr<Frame> frameReference = nullptr;
	auto bGlobalizeReference = false;
	if(refId == -1 || bHasReferenceAnim == false)
	{
		std::cout<<"WARNING: No reference animation found; Attempting to generate reference from default bone transforms..."<<std::endl;
		reference = std::make_shared<Animation>();
		mdl.AddAnimation("reference",reference);
	}
	else
	{
		reference = mdl.GetAnimation(refId);
		//frameReference = reference->GetFrame(0);
		//bGlobalizeReference = true;
	}
	reference->GetFrames().clear();

	std::vector<uint32_t> refBoneList;
	refBoneList.reserve(header.numbones);
	for(auto i=decltype(header.numbones){0};i<header.numbones;++i)
		refBoneList.push_back(i);
	reference->SetBoneList(refBoneList);

	frameReference = std::make_shared<Frame>(header.numbones);
	for(auto i=decltype(header.numbones){0};i<header.numbones;++i)
	{
		const auto fAngToMat = [](const EulerAngles &ang) -> Mat4 {
			Mat4 mat(1.0f);
			mat = glm::rotate(mat,CFloat(umath::deg_to_rad(ang.r)),uvec::RIGHT);
			mat = glm::rotate(mat,CFloat(umath::deg_to_rad(ang.y)),uvec::UP);
			mat = glm::rotate(mat,CFloat(umath::deg_to_rad(ang.p)),uvec::FORWARD);
			return mat;
		};

		auto &origBone = bones[i];
		auto &pos = origBone->GetPos();
		auto &ang = origBone->GetAngles();
		//auto rot = origBone->GetRot();//glm::toQuat(fAngToMat(ang));
		//auto m = mdl::util::euler_angles_to_matrix(ang);
		//auto rot = mdl::util::get_rotation(m);
		//auto rot = Quat{0.f,ang.x,ang.y,ang.z};
		//uquat::normalize(rot);
		//auto rot = Quat{0.0,ang.x,ang.y,ang.z};
		auto rot = import::mdl::quaternion_to_euler_angles(origBone->GetRot());
		
		frameReference->SetBonePosition(i,{-pos.x,pos.y,pos.z});
		frameReference->SetBoneOrientation(i,Quat{0.0,rot.x,rot.y,rot.z});//rot);
	}
	reference->AddFrame(frameReference);

	/*auto &anims = mdl.GetAnimations();
	//for(auto i=decltype(anims.size()){0};i<anims.size() -1;++i) // Last animation is reference (TODO: Is that a problem?)
	{
		auto i = mdl.LookupAnimation("attack1");
	//	auto rotAdd = uquat::create(EulerAngles(90,180.f,0));
		auto &anim = anims[i];
		for(auto &frame : anim->GetFrames())
		{
			frame->Globalize(anim.get(),&skeleton);
			for(auto j=decltype(header.numbones){0};j<header.numbones;++j)
			{
				auto &pos = *frame->GetBonePosition(j);
				//uvec::rotate(&pos,rotAdd);
				auto &rot = *frame->GetBoneOrientation(j);
				//rot = rotAdd *rot;
				//rot.w = -rot.w;
				//rot.y = -rot.y;
			}
			frame->Localize(anim.get(),&skeleton);
		}
	}*/


	/*auto bFirst = true;
	for(auto &anim : mdl.GetAnimations())
	{
		if(bFirst == true)
		{
			bFirst = false;
			continue;
		}
		for(auto &frame : anim->GetFrames())
		{
			for(auto i=0;i<header.numbones;++i)
			{
				frame->SetBonePosition(i,*refPose->GetBonePosition(i));
				frame->SetBoneOrientation(i,*refPose->GetBoneOrientation(i));
			}
			//frame->Localize(anim.get(),&skeleton);
		}
	}*/ // ????????

	//auto mesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
	//auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
	//auto &triangles = subMesh->GetTriangles();
	//auto &verts = subMesh->GetVertices();
	//auto &vertWeights = subMesh->GetVertexWeights();

	std::vector<Vertex> vvdVerts;
	std::vector<VertexWeight> vvdVertWeights;
	it = files.find("vvd");
	if(it != files.end())
	{
		import::mdl::load_vvd(it->second,vvdVerts,vvdVertWeights,mdlInfo.fixedLodVertexIndices);
		if(mdlInfo.header.flags &STUDIOHDR_FLAGS_STATIC_PROP)
		{
			for(auto &v : vvdVerts)
			{
				umath::swap(v.position.x,v.position.y);
				umath::negate(v.position.y);

				umath::swap(v.normal.x,v.normal.y);
				umath::negate(v.normal.y);
			}
		}
	}

	struct MeshBodyPartInfo
	{
		MeshBodyPartInfo(ModelMeshGroup &mg,ModelMesh &m,ModelSubMesh &sm,uint32_t bpIdx,uint32_t mIdx,uint32_t msIdx)
			: meshGroup(mg),mesh(m),subMesh(sm),bodyPartIdx(bpIdx),mdlIdx(mIdx),meshIdx(msIdx)
		{}
		ModelMeshGroup &meshGroup;
		ModelMesh &mesh;
		ModelSubMesh &subMesh;
		uint32_t bodyPartIdx = std::numeric_limits<uint32_t>::max();
		uint32_t mdlIdx = std::numeric_limits<uint32_t>::max();
		uint32_t meshIdx = std::numeric_limits<uint32_t>::max();
		std::unordered_map<uint16_t,uint16_t> bpMeshIndicesToMeshIndices;
	};
	auto &bodyParts = mdlInfo.bodyParts;
	std::vector<MeshBodyPartInfo> meshToBodyPart {};
	std::vector<mdl::vtx::VtxBodyPart> vtxBodyParts;
	it = files.find("vtx");
	std::vector<std::shared_ptr<ModelSubMesh>> skipSharedMeshTransforms {}; // Skip vertex transformations for these meshes, because they use shared vertices from other meshes

	// TODO: Add LOD info
	std::vector<uint32_t> bgBaseMeshGroups = {};
	if(it != files.end())
	{
		auto bVTX = import::mdl::load_vtx(it->second,vtxBodyParts);
		if(bVTX == true && !vtxBodyParts.empty())
		{
			uint32_t bodyPartVertexIndexStart = 0;
			for(auto bodyPartIdx=decltype(vtxBodyParts.size()){0};bodyPartIdx<vtxBodyParts.size();++bodyPartIdx)
			{
				auto &vtxBodyPart = vtxBodyParts[bodyPartIdx]; // aBodyPart
				auto &vtxModels = vtxBodyPart.models;

				auto &bodyPart = bodyParts[bodyPartIdx];
				auto &models = bodyPart.GetModels();

				auto &bgName = bodyPart.GetName();
				auto &bgroup = mdl.AddBodyGroup(bgName);
				for(auto mdlIdx=decltype(vtxModels.size()){0};mdlIdx<vtxModels.size();++mdlIdx)
				{
					auto &vtxModel = vtxModels[mdlIdx]; // aVtxModel
					auto &model = models[mdlIdx];

					std::shared_ptr<ModelSubMesh> rootMesh = nullptr;
					for(auto lodIdx=decltype(vtxModel.lods.size()){0};lodIdx<vtxModel.lods.size();++lodIdx)
					{
						auto bgLogName = bgName +"_" +((lodIdx > 0) ? (std::string("lod") +std::to_string(lodIdx)) : "reference");
						auto subId = 2u;
						while(mdl.GetMeshGroup(bgLogName) != nullptr)
							bgLogName = bgName +std::to_string(subId++) +"_" +((lodIdx > 0) ? (std::string("lod") +std::to_string(lodIdx)) : "reference");
						uint32_t meshGroupId = std::numeric_limits<uint32_t>::max();
						auto meshGroup = mdl.AddMeshGroup(bgLogName,meshGroupId);
						if(lodIdx == 0u)
						{
							auto it = std::find(bgroup.meshGroups.begin(),bgroup.meshGroups.end(),meshGroupId);
							if(it == bgroup.meshGroups.end())
								bgroup.meshGroups.push_back(meshGroupId);
							if(mdlIdx == 0u)
							{
								it = std::find(bgBaseMeshGroups.begin(),bgBaseMeshGroups.end(),meshGroupId);
								if(it == bgBaseMeshGroups.end())
									bgBaseMeshGroups.push_back(meshGroupId);
							}
						}
						auto lodMesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());

						auto &vtxLod = vtxModel.lods[lodIdx];
						for(auto meshIdx=decltype(vtxLod.meshes.size()){0};meshIdx<vtxLod.meshes.size();++meshIdx)
						{
							auto lodSubMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
							auto &verts = lodSubMesh->GetVertices();
							auto &vertWeights = lodSubMesh->GetVertexWeights();
							auto &triangles = lodSubMesh->GetTriangles();
							std::unordered_map<uint16_t,uint16_t> newIndices;

							auto &vtxMesh = vtxLod.meshes[meshIdx];
							auto &mesh = model.meshes[meshIdx];

							std::unordered_map<uint16_t,uint16_t> *bpMeshIndexToMeshIndices = nullptr;
							if(lodIdx == 0u)
							{
								if(meshToBodyPart.size() == meshToBodyPart.capacity())
									meshToBodyPart.reserve(meshToBodyPart.size() +50);
								meshToBodyPart.push_back({*meshGroup,*lodMesh,*lodSubMesh,static_cast<uint32_t>(bodyPartIdx),static_cast<uint32_t>(mdlIdx),static_cast<uint32_t>(meshIdx)});
								bpMeshIndexToMeshIndices = &meshToBodyPart.back().bpMeshIndicesToMeshIndices;
							}

							lodSubMesh->SetTexture(mesh.stdMesh.material);
							auto meshVertexIndexStart = mesh.stdMesh.vertexoffset;
							for(auto &stripGroup : vtxMesh.stripGroups)
							{
								triangles.reserve(triangles.size() +stripGroup.indices.size());
								for(auto i=decltype(stripGroup.indices.size()){0};i<stripGroup.indices.size();i++)
								{
									auto vtxIdx0 = stripGroup.indices[i];
									auto &vtxVert = stripGroup.stripVerts[vtxIdx0];
									auto vertIdx = vtxVert.origMeshVertID +meshVertexIndexStart +bodyPartVertexIndexStart;
									auto originalIdx = vertIdx;
									if(mdlInfo.fixedLodVertexIndices.empty() == false)
									{
										auto &lodVertIndices = mdlInfo.fixedLodVertexIndices.front();
										vertIdx = lodVertIndices[vertIdx];
									}
									auto fixedIdx = vertIdx;
									auto it = newIndices.find(vertIdx);
									if(it == newIndices.end()) // New unique vertex
									{
										auto &vvdVert = vvdVerts[vertIdx];
										auto &vvdWeight = vvdVertWeights[vertIdx];
										verts.push_back(vvdVert);
										vertWeights.push_back(vvdWeight);

										vertIdx = newIndices[vertIdx] = verts.size() -1;
									}
									else
										vertIdx = it->second;
									if(bpMeshIndexToMeshIndices != nullptr)
									{
										//(*bpMeshIndexToMeshIndices)[originalIdx] = vertIdx;
										(*bpMeshIndexToMeshIndices)[vtxVert.origMeshVertID] = vertIdx;
										//if(bodyPartIdx == 0 && mdlIdx == 0 && meshIdx == 1 && (fixedIdx == 301 || vtxVert.origMeshVertID == 301 || originalIdx == 301 || vtxVert.origMeshVertID +meshVertexIndexStart == 301 || vtxVert.origMeshVertID +meshVertexIndexStart +bodyPartVertexIndexStart == 301))
										//	std::cout<<"!!!"<<std::endl;
									}
									triangles.push_back(vertIdx);
								}
							}

							// Swap triangle winding order
							for(auto i=decltype(triangles.size()){};i<triangles.size();i+=3)
								umath::swap(triangles[i],triangles[i +1]);

							lodMesh->AddSubMesh(lodSubMesh);
							lodSubMesh->Update();
						}
						meshGroup->AddMesh(lodMesh);
						lodMesh->Update();

						if(lodIdx > 0)
						{
							std::unordered_map<uint32_t,uint32_t> meshIds = {};
							auto meshGroupIdx = lodIdx;
							meshIds[0] = meshGroupIdx;
							mdl.AddLODInfo(lodIdx,meshIds);
						}
					}
					bodyPartVertexIndexStart += model.vertexCount;
				}
			}
			/*for(auto lod=decltype(bodyPart.lodMeshes.size()){0};lod<bodyPart.lodMeshes.size();++lod)
			{
				auto &bodyPartLod = bodyPart.lodMeshes[lod];

				auto meshGroup = mdl.AddMeshGroup((lod > 0) ? (std::string("lod") +std::to_string(lod)) : "reference");
				auto lodMesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
				auto lodSubMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());

				lodSubMesh->GetTriangles() = bodyPartLod.indices;
				if(mdlInfo.fixedLodVertexIndices.empty())
				{
					if(lod == 0)
					{
						lodSubMesh->GetVertices() = vvdVerts;
						lodSubMesh->GetVertexWeights() = vvdVertWeights;
						rootMesh = lodSubMesh;
					}
					else
					{
						lodSubMesh->SetShared(*rootMesh,ModelSubMesh::ShareMode::All & ~ModelSubMesh::ShareMode::Triangles);
						skipSharedMeshTransforms.push_back(lodSubMesh);
					}
				}
				else
				{
					auto &fixedVerts = mdlInfo.fixedLodVertexIndices[lod];
					auto &verts = lodSubMesh->GetVertices();
					auto &vertWeights= lodSubMesh->GetVertexWeights();
					verts.reserve(fixedVerts.size());
					vertWeights.reserve(fixedVerts.size());
					for(auto idx : fixedVerts)
					{
						verts.push_back(vvdVerts[idx]);
						vertWeights.push_back(vvdVertWeights[idx]);
					}
				}
				
				lodSubMesh->SetTexture(0); // TODO? CHECK MULTIPLE TEX MODELS
				lodMesh->AddSubMesh(lodSubMesh);
				meshGroup->AddMesh(lodMesh);

				if(lod > 0)
				{
					std::unordered_map<uint32_t,uint32_t> meshIds = {};
					meshIds[0] = lod;
					mdl.AddLODInfo(lod,meshIds);
				}
				lodSubMesh->Update();
				lodMesh->Update();
			}*/
		}
	}

	// Vertex Animations
	struct PairInfo
	{
		PairInfo(ModelMesh &m,ModelSubMesh &sm)
			: mesh(m),subMesh(sm)
		{}
		ModelMesh &mesh;
		ModelSubMesh &subMesh;
		uint32_t pairId = std::numeric_limits<uint32_t>::max();
		std::vector<Vector3> pairTransforms = {};
	};
	std::unordered_map<uint32_t,PairInfo> flexPairs;
	auto bpIdx = 0u;
	for(auto &bp : mdlInfo.bodyParts)
	{
		auto mdlIdx = 0u;
		for(auto &bpMdl : bp.GetModels())
		{
			auto meshIdx = 0u;
			for(auto &mesh : bpMdl.meshes)
			{
				auto it = std::find_if(meshToBodyPart.begin(),meshToBodyPart.end(),[bpIdx,mdlIdx,meshIdx](const MeshBodyPartInfo &info) {
					return (bpIdx == info.bodyPartIdx && mdlIdx == info.mdlIdx && meshIdx == info.meshIdx) ? true : false;
				});
				if(it == meshToBodyPart.end())
				{
					++meshIdx;
					continue;
				}
				auto &info = *it;
				auto &meshFlexes = mesh.flexes;
				for(auto &flex : meshFlexes)
				{
					auto &stdFlex = flex.stdFlex;
					auto &vertexTransforms = flex.vertAnim;

					auto &stdFlexDesc = mdlInfo.flexDescs.at(stdFlex.flexdesc);
					auto vertAnim = mdl.AddVertexAnimation("flex_" +stdFlexDesc.GetName());
					auto meshFrame = vertAnim->AddMeshFrame(info.mesh,info.subMesh);
					if(stdFlex.flexdesc < flexes.size())
					{
						auto &flex = flexes.at(stdFlex.flexdesc);
						flex.SetVertexAnimation(*vertAnim,*vertAnim->GetMeshAnimation(info.subMesh),*meshFrame);
					}

					auto *flexLowerer = mdlInfo.model.GetFlex(stdFlexDesc.GetName() +"_lowerer");
					auto *flexRaiser = mdlInfo.model.GetFlex(stdFlexDesc.GetName() +"_raiser");
					if(flexLowerer != nullptr && flexRaiser != nullptr) // Probably an eyelid?
					{
						// Assign frame 0 to lowerer and frame 1 to raiser, that seems to do the trick
						auto &meshAnim = *vertAnim->GetMeshAnimation(info.subMesh);
						if(flexLowerer != nullptr && meshAnim.GetFrames().size() >= 1)
							flexLowerer->SetVertexAnimation(*vertAnim,meshAnim,*meshAnim.GetFrame(0));

						if(flexRaiser != nullptr && meshAnim.GetFrames().size() >= 2)
							flexRaiser->SetVertexAnimation(*vertAnim,meshAnim,*meshAnim.GetFrame(1));
					}
					/*auto *flexNeutral = mdlInfo.model.GetFlex(stdFlexDesc.GetName() +"_neutral");
					if(flexNeutral != nullptr)
					{
						auto &meshAnim = *vertAnim->GetMeshAnimation(info.subMesh);
						if(flexNeutral != nullptr && meshAnim.GetFrames().size() >= 2)
							flexNeutral->SetVertexAnimation(*vertAnim,meshAnim,*meshAnim.GetFrame(1));
					}*/

					VertexAnimation *pairVertAnim = nullptr;
					MeshVertexFrame *pairMeshFrame = nullptr;
					if(stdFlex.flexpair != 0)
					{
						auto &stdFlexDescPair = mdlInfo.flexDescs.at(stdFlex.flexpair);
						pairVertAnim = mdl.AddVertexAnimation("flex_" +stdFlexDescPair.GetName()).get();
						pairMeshFrame = pairVertAnim->AddMeshFrame(info.mesh,info.subMesh).get();
						if(stdFlex.flexpair < flexes.size())
						{
							auto &flex = flexes.at(stdFlex.flexpair);
							flex.SetVertexAnimation(*pairVertAnim,*pairVertAnim->GetMeshAnimation(info.subMesh),*pairMeshFrame);
						}
					}
					
					for(auto &t : vertexTransforms)
					{
						auto it = info.bpMeshIndicesToMeshIndices.find(t->index);
						if(it != info.bpMeshIndicesToMeshIndices.end())
						{
							auto side = t->side /255.f;
							auto scale = 1.f -side;

							Vector3 v{float16::Convert16bitFloatTo32bits(t->flDelta.at(1)),float16::Convert16bitFloatTo32bits(t->flDelta.at(2)),float16::Convert16bitFloatTo32bits(t->flDelta.at(0))};
							if(pairMeshFrame != nullptr)
							{
								pairMeshFrame->SetVertexPosition(it->second,v *(1.f -scale));
								v *= scale; // TODO: Unsure if this is correct
							}
							meshFrame->SetVertexPosition(it->second,v);
						}
						else
							std::cout<<"WARNING: Missing flex vertex "<<t->index<<" for flex "<<stdFlexDesc.GetName()<<"! Skipping..."<<std::endl;
					}
				}
				++meshIdx;
			}
			++mdlIdx;
		}
		++bpIdx;
	}

	//for(auto &pair : flexPairs)
	//{
	//	auto &pairInfo = pair.second; // TODO: Use these vertices
	//	auto &stdFlexDesc0 = mdlInfo.flexDescs.at(pair.first);
	//	auto &stdFlexDesc1 = mdlInfo.flexDescs.at(pairInfo.pairId);
	//	auto va1 = mdl.AddVertexAnimation(stdFlexDesc1.GetName());
	//	auto *va0 = mdl.GetVertexAnimation(stdFlexDesc0.GetName());
	//	if(va0 == nullptr || va1 == nullptr)
	//		continue;
	//	auto &meshAnims1 = va1->GetMeshAnimations();
	//	auto maIdx = 0u;
	//	for(auto &ma0 : (*va0)->GetMeshAnimations())
	//	{
	//		auto frameIdx = 0u;
	//		for(auto &frame0 : ma0->GetFrames())
	//		{
	//			std::shared_ptr<MeshVertexFrame> frame1 = nullptr;
	//			if(maIdx < meshAnims1.size())
	//			{
	//				auto &ma1 = meshAnims1.at(maIdx);
	//				auto &frames1 = ma1->GetFrames();
	//				if(frameIdx < frames1.size())
	//					frame1 = frames1.at(frameIdx);
	//			}
	//			if(frame1 == nullptr)
	//				frame1 = va1->AddMeshFrame(*ma0->GetMesh(),*ma0->GetSubMesh());

	//			auto numVerts = umath::min(frame0->GetVertices().size(),frame1->GetVertices().size());
	//			for(auto i=decltype(numVerts){0};i<numVerts;++i)
	//			{
	//				Vector3 v0;
	//				Vector3 v1;
	//				if(frame0->GetVertexPosition(i,v0) == false || frame1->GetVertexPosition(i,v1) == false)
	//					continue;
	//				frame1->SetVertexPosition(i,v0);
	//				/*if(v0.x < 0.f)
	//					frame1->SetVertexPosition(i,Vector3{});
	//				else
	//				{
	//					frame1->SetVertexPosition(i,v0);
	//					frame0->SetVertexPosition(i,Vector3{});
	//				}*/
	//			}
	//			++frameIdx;
	//		}
	//		++maIdx;
	//	}
	//}
	//


	/*if(mdlInfo.fixedLodVertexIndices.empty())
	{
		verts = vvdVerts;
		vertWeights = vvdVertWeights;
	}
	else
	{
		auto &fixedVerts = mdlInfo.fixedLodVertexIndices.front(); // TODO: What about other LODs?
		verts.reserve(fixedVerts.size());
		vertWeights.reserve(fixedVerts.size());
		for(auto idx : fixedVerts)
		{
			verts.push_back(vvdVerts[idx]);
			vertWeights.push_back(vvdVertWeights[idx]);
		}
	}*/

	std::unordered_map<std::string,std::vector<std::pair<std::string,float>>> defaultPhonemes = {
			{"sh",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.400000006},
					{"left_funneler",0.400000006},
					{"right_funneler",0.400000006},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"<sil>",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"ow",{
					{"left_puckerer",0.6000000238},
					{"right_puckerer",0.6000000238},
					{"bite",0},
					{"left_mouth_drop",0.5400000215},
					{"right_mouth_drop",0.5400000215},
					{"tightener",0.5299999714},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.4699999988},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"r2",{
					{"left_puckerer",0.75},
					{"right_puckerer",0.75},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.25},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"th",{
					{"puckerer",0},
					{"bite",0.1000000015},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.1299999952},
					{"left_funneler",0.5},
					{"right_funneler",0.5},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",0.2899999917},
					{"right_part",0.2899999917},
					{"chin_raiser",0.2599999905},
			}},
			{"b",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",1},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.1000000015},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"r3",{
					{"left_puckerer",1},
					{"right_puckerer",1},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.25},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"iy",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"left_stretcher",0.7799999714},
					{"right_stretcher",0.7799999714},
					{"presser",0},
					{"jaw_clencher",0.5500000119},
					{"left_funneler",0.375},
					{"right_funneler",0.375},
					{"jaw_drop",0.150000006},
					{"upper_raiser",0},
					{"left_part",1},
					{"right_part",1},
					{"chin_raiser",0},
			}},
			{"r",{
					{"left_puckerer",0.5099999905},
					{"right_puckerer",0.5099999905},
					{"bite",0},
					{"left_mouth_drop",0.0500000007},
					{"right_mouth_drop",0.0500000007},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"t",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"left_stretcher",0.4399999976},
					{"right_stretcher",0.4399999976},
					{"presser",0},
					{"jaw_clencher",1},
					{"left_funneler",0.2199999988},
					{"right_funneler",0.2199999988},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",1},
					{"right_part",1},
					{"chin_raiser",0},
			}},
			{"ao",{
					{"left_puckerer",0.5},
					{"right_puckerer",0.5},
					{"bite",0},
					{"left_mouth_drop",0.6000000238},
					{"right_mouth_drop",0.6000000238},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.25},
					{"right_funneler",0.25},
					{"jaw_drop",0.375},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"aa2",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.5},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"n",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"left_stretcher",0.3000000119},
					{"right_stretcher",0.3000000119},
					{"presser",0},
					{"jaw_clencher",0.200000003},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",0.8899999857},
					{"right_part",0.8899999857},
					{"chin_raiser",0},
			}},
			{"aa",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.400000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"g",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.150000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"w",{
					{"left_puckerer",0.8000000119},
					{"right_puckerer",0.8000000119},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0.4499999881},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.1000000015},
					{"right_funneler",0.075000003},
					{"jaw_drop",0.3249999881},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"d2",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.4499999881},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",1},
					{"right_part",1},
					{"chin_raiser",0},
			}},
			{"ih2",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",0.8799999952},
					{"right_mouth_drop",0.8799999952},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.3249999881},
					{"upper_raiser",0},
					{"left_part",0.400000006},
					{"right_part",0.400000006},
					{"chin_raiser",0},
			}},
			{"hh2",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",0.6499999762},
					{"right_mouth_drop",0.6499999762},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.5},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"f",{
					{"puckerer",0},
					{"bite",0.7400000095},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.1000000015},
					{"right_funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"y",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0.349999994},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.3000000119},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",1},
					{"right_part",1},
					{"chin_raiser",0},
			}},
			{"v",{
					{"puckerer",0},
					{"bite",1},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"zh",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.5500000119},
					{"left_funneler",0.5},
					{"right_funneler",0.5},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"nx",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"left_stretcher",0.400000006},
					{"right_stretcher",0.400000006},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.1000000015},
					{"upper_raiser",0},
					{"left_part",0.4499999881},
					{"right_part",0.4499999881},
					{"chin_raiser",0},
			}},
			{"c",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.25},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",0.8500000238},
					{"right_part",0.8500000238},
					{"chin_raiser",0},
			}},
			{"ey",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"left_stretcher",0.3000000119},
					{"right_stretcher",0.3000000119},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.349999994},
					{"right_funneler",0.3249999881},
					{"jaw_drop",0.150000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"g2",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.150000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"hh",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",0.5},
					{"right_mouth_drop",0.5},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.400000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"eh",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"left_stretcher",0.6000000238},
					{"right_stretcher",0.6000000238},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.25},
					{"right_funneler",0.25},
					{"jaw_drop",0.200000003},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"jh",{
					{"left_puckerer",1},
					{"right_puckerer",1},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.349999994},
					{"left_funneler",0.150000006},
					{"right_funneler",0.150000006},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",1},
					{"right_part",1},
					{"chin_raiser",0},
			}},
			{"z",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"left_stretcher",0.5},
					{"right_stretcher",0.5},
					{"presser",0},
					{"jaw_clencher",0.9900000095},
					{"funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",1},
					{"right_part",1},
					{"chin_raiser",0},
			}},
			{"d",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.25},
					{"right_funneler",0.25},
					{"jaw_drop",0.1000000015},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"ih",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.200000003},
					{"upper_raiser",0},
					{"left_part",0.400000006},
					{"right_part",0.400000006},
					{"chin_raiser",0},
			}},
			{"m",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",1},
					{"jaw_clencher",0},
					{"funneler",0.1000000015},
					{"jaw_drop",0.150000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"ch",{
					{"left_puckerer",0.9499999881},
					{"right_puckerer",0.9499999881},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0.6499999762},
					{"left_funneler",0.400000006},
					{"right_funneler",0.400000006},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"ax",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",0.5},
					{"right_mouth_drop",0.5},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.25},
					{"right_funneler",0.25},
					{"jaw_drop",0.400000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"ae",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",0.8000000119},
					{"right_mouth_drop",0.8000000119},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.349999994},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"l2",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.150000006},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"er",{
					{"left_puckerer",0.9300000072},
					{"right_puckerer",0.9300000072},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.3799999952},
					{"right_funneler",0},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"left_part",0.6000000238},
					{"right_part",0.6000000238},
					{"chin_raiser",0},
			}},
			{"uw",{
					{"left_puckerer",0.9499999881},
					{"right_puckerer",0.8999999762},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.200000003},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"ah",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.3000000119},
					{"upper_raiser",0},
					{"left_part",0.25},
					{"right_part",0.25},
					{"chin_raiser",0},
			}},
			{"ax2",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",0.5099999905},
					{"right_mouth_drop",0.5099999905},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.25},
					{"right_funneler",0.25},
					{"jaw_drop",0.5},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"l",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",1},
					{"right_mouth_drop",1},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.150000006},
					{"upper_raiser",0},
					{"left_part",0.5400000215},
					{"right_part",0.5400000215},
					{"chin_raiser",0},
			}},
			{"dh",{
					{"puckerer",0},
					{"bite",0},
					{"left_mouth_drop",0.75},
					{"right_mouth_drop",0.75},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.200000003},
					{"right_funneler",0.200000003},
					{"jaw_drop",0.150000006},
					{"left_upper_raiser",0.150000006},
					{"right_upper_raiser",0.150000006},
					{"left_part",0.400000006},
					{"right_part",0.400000006},
					{"chin_raiser",0},
			}},
			{"uh",{
					{"left_puckerer",0.75},
					{"right_puckerer",0.75},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"funneler",0},
					{"jaw_drop",0.224999994},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"er2",{
					{"left_puckerer",0.5},
					{"right_puckerer",0.5},
					{"bite",0},
					{"left_mouth_drop",0.5},
					{"right_mouth_drop",0.5},
					{"tightener",0},
					{"stretcher",0},
					{"presser",0},
					{"jaw_clencher",0},
					{"left_funneler",0.25},
					{"right_funneler",0.25},
					{"jaw_drop",0.25},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"s",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"left_stretcher",0.6999999881},
					{"right_stretcher",0.6999999881},
					{"presser",0},
					{"jaw_clencher",0.5500000119},
					{"left_funneler",0.375},
					{"right_funneler",0.375},
					{"jaw_drop",0},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
			{"p",{
					{"puckerer",0},
					{"bite",0},
					{"mouth_drop",0},
					{"tightener",0},
					{"stretcher",0},
					{"presser",1},
					{"jaw_clencher",0},
					{"funneler",0.1000000015},
					{"jaw_drop",0.1000000015},
					{"upper_raiser",0},
					{"part",0},
					{"chin_raiser",0},
			}},
	};
	auto &phonemeMap = mdlInfo.model.GetPhonemeMap();
	for(auto &pair : defaultPhonemes)
	{
		auto &flexControllers = phonemeMap.phonemes.insert(std::make_pair(pair.first,PhonemeInfo{})).first->second.flexControllers;
		flexControllers.reserve(pair.second.size());
		for(auto &pair : pair.second)
			flexControllers.insert(pair);
	}

	// Transform coordinate systems
	mdlInfo.ConvertTransforms(skipSharedMeshTransforms,reference.get());
	//

	// Load physics
	std::vector<mdl::phy::PhyCollisionData> collisionObjects {};
	std::unordered_map<int32_t,mdl::phy::PhyRagdollConstraint> constraints {};
	it = files.find("phy");
	if(it != files.end())
	{
		if(import::mdl::load_phy(it->second,collisionObjects,constraints) == true)
		{
			auto bFirst = true;
			auto colMeshOffset = mdl.GetCollisionMeshCount();
			for(auto &colObj : collisionObjects)
			{
				auto colMesh = CollisionMesh::Create(nw->GetGameState());

				int32_t boneId = -1;
				std::shared_ptr<mdl::Bone> bone = nullptr;
				if(!colObj.faceSections.empty())
				{
					auto &face = colObj.faceSections.front();
					boneId = face.boneIdx;
					bone = bones[boneId]; // TODO Faces with different bones?
				}
				auto &verts = colMesh->GetVertices();
				verts.reserve(colObj.vertices.size());

				// Transformation
				/*auto pos = Vector3{}; // firstAnimDescFrameLine.position;
				auto rot = Vector3{}; // firstAnimDescFrameLine.rotation

				auto mat = EulerAngles{rot.x,rot.y,rot.z +umath::deg_to_rad(-90.f)}.ToMatrix();
				mat = glm::translate(Vector3{pos.y,-pos.x,pos.z}) *mat;
				// TODO: Verify that order is correct (position not rotated!)

				auto fMatrixInvert = [](const Mat4 &in,Mat4 &out) {
					out[0][0] = in[0][0];
					out[1][0] = in[0][1];
					out[2][0] = in[0][2];

					out[0][1] = in[1][0];
					out[1][1] = in[1][1];
					out[2][1] = in[1][2];

					out[0][2] = in[2][0];
					out[1][2] = in[2][1];
					out[2][2] = in[2][2];

					Vector3 temp {};
					temp.x = in[3][0];
					temp.y = in[3][1];
					temp.z = in[3][2];

					out[3][0] = -uvec::dot(temp,Vector3{out[0][0],out[1][0],out[2][0]});
					out[3][1] = -uvec::dot(temp,Vector3{out[0][1],out[1][1],out[2][1]});
					out[3][2] = -uvec::dot(temp,Vector3{out[0][2],out[1][2],out[2][2]});
				};
				Mat4 invMat {};
				fMatrixInvert(mat,invMat);*/
				// ???????
				// See
				// Public Sub MatrixInvert(ByVal in_matrixColumn0 As SourceVector, ByVal in_matrixColumn1 As SourceVector, ByVal in_matrixColumn2 As SourceVector, ByVal in_matrixColumn3 As SourceVector, ByRef out_matrixColumn0 As SourceVector, ByRef out_matrixColumn1 As SourceVector, ByRef out_matrixColumn2 As SourceVector, ByRef out_matrixColumn3 As SourceVector)
				// SourceSmdFile44.vb -> Private Sub ProcessTransformsForPhysics()
				//



				for(auto &v : colObj.vertices)
				{
					auto vpos = v.first;
					if(boneId == -1 || bones.size() <= 1) // TODO Seems to work for most models, but why is this needed?
					{
						const auto v = static_cast<float>(umath::sin(M_PI_4));
						const auto rot = Quat{0.f,v,0.f,v}; // Rotation 180 degree on pitch axis and -90 degree on yaw axis
						//vpos = vpos *rot;
					}
					if(colObj.sourcePhyIsCollision == true)
					{
						auto rot180 = uquat::create(EulerAngles(0,180,0)); // Rotation by 180 degree on yaw-axis; Works on most props, not sure why this is needed
						//vpos = vpos *rot180;
					}

					auto t = mdl::util::transform_physics_vertex(bone,vpos,colObj.sourcePhyIsCollision);
					if(colObj.sourcePhyIsCollision == true)
					{
						boneId = -1;
						umath::swap(t.y,t.z);
						umath::negate(t.z);

						umath::negate(t.y);
						// boneId == 0 is meaningless if sourcePhyIsCollision is true (= collision mesh has NO parent)
						//umath::swap(t.y,t.z);
						//umath::negate(t.z);
						/*if(mdlInfo.header.flags &STUDIOHDR_FLAGS_STATIC_PROP)
						{
							uvec::rotate(&t,uquat::create(EulerAngles(0.f,-90.f,0.f)));

							auto rot = uquat::create(EulerAngles(0,0,-90));
							uvec::rotate(&t,rot);
							t.x = -t.x;

							uvec::rotate(&t,uquat::get_inverse(uquat::create(EulerAngles(0.f,-90.f,0.f))));
						}
						else if(boneId == -1 || bones.size() <= 1)
						{
							umath::swap(t.y,t.z);
							umath::negate(t.z);
							t.y = -t.y;
						}
						else
						{
							uvec::rotate(&t,uquat::create(EulerAngles(0.f,-90.f,0.f)));

							auto rot = uquat::create(EulerAngles(0,90,-90));
							uvec::rotate(&t,rot);
							t.y = -t.y;

							uvec::rotate(&t,uquat::get_inverse(uquat::create(EulerAngles(0.f,-90.f,0.f))));
						}*/
					}
					else
					{
						umath::swap(t.y,t.z);
						umath::negate(t.z);
					}
					
					// Perfect fit for phycannon:
					//umath::negate(t.y);

					// Perfect fit for smg:
					//umath::negate(t.x);
					//umath::negate(t.y);
					//umath::negate(t.z);

					verts.push_back(t);
				}

				// There are very few models where the physics mesh will be rotated incorrectly by either 90 or 180 degrees on some axes for whatever reason. (e.g. weapons/w_smg1.mdl)
				// There seems to be no obvious way to find the correct rotation, so the code below will just rotate the collision mesh by all posibilities and select the one which
				// encompasses the visible geometry the closest - which is usually the correct one.
				// If it's not the correct one, the model will have to be rotated manually.
				if(colObj.sourcePhyIsCollision == true)
				{
					const std::array<float,5> testAngles = {-180.f,-90.f,0.f,90.f,180.f};
					auto smallestDistanceSum = std::numeric_limits<float>::max();
					auto smallestCandidate = EulerAngles(0,0,0);
					for(auto p : testAngles)
					{
						for(auto y : testAngles)
						{
							for(auto r : testAngles)
							{
								auto ang = EulerAngles(p,y,r);
								auto rot = uquat::create(ang);
								auto distSum = 0;
								for(auto v : verts)
								{
									uvec::rotate(&v,rot);
									auto dClosest = std::numeric_limits<float>::max();
									auto meshGroup = mdl.GetMeshGroup(0);
									if(meshGroup != nullptr)
									{
										for(auto &mesh : meshGroup->GetMeshes())
										{
											for(auto &subMesh : mesh->GetSubMeshes())
											{
												auto &triangles = subMesh->GetTriangles();
												auto &verts = subMesh->GetVertices();
												for(auto i=decltype(triangles.size()){0};i<triangles.size();i+=3)
												{
													auto &v0 = verts.at(triangles.at(i));
													auto &v1 = verts.at(triangles.at(i +1));
													auto &v2 = verts.at(triangles.at(i +2));
													Vector3 r;
													Geometry::ClosestPointOnTriangleToPoint(v0.position,v1.position,v2.position,v,&r);
													auto d = uvec::distance(v,r);
													if(d < dClosest)
														dClosest = d;
												}
											}
										}
									}
									distSum += dClosest;
								}
								if(distSum < smallestDistanceSum)
								{
									smallestDistanceSum = distSum;
									smallestCandidate = ang;
								}
								else if(distSum < smallestDistanceSum +0.1f) // Approximately equal; Use whichever angle-set requires the least amount of rotations
								{
									if((umath::abs(ang.p) +umath::abs(ang.y) +umath::abs(ang.r)) < (umath::abs(smallestCandidate.p) +umath::abs(smallestCandidate.y) +umath::abs(smallestCandidate.r)))
									{
										smallestDistanceSum = distSum;
										smallestCandidate = ang;
									}
								}
							}
						}
					}
					if(smallestDistanceSum != std::numeric_limits<float>::max() && smallestCandidate != EulerAngles(0,0,0))
					{
						auto rot = uquat::create(smallestCandidate);
						for(auto &v : verts)
							uvec::rotate(&v,rot);
					}
				}
				
				colMesh->Centralize();
				colMesh->SetSurfaceMaterial(colObj.keyValues.surfaceProp);
				colMesh->SetBoneParent(boneId);
				colMesh->Update();
				colMesh->SetVolume(colObj.keyValues.volume);
				mdl.AddCollisionMesh(colMesh);
				if(bFirst == true)
				{
					bFirst = false;
					mdl.SetMass(colObj.keyValues.mass);
				}
			}
			for(auto &pair : constraints)
			{
				auto &ragdollConstraint = pair.second;

				auto &joint = mdl.AddJoint(JOINT_TYPE_DOF,ragdollConstraint.childIndex,ragdollConstraint.parentIndex);

				auto angLimitL = EulerAngles(ragdollConstraint.xmin,-ragdollConstraint.zmax,ragdollConstraint.ymin);
				auto angLimitU = EulerAngles(ragdollConstraint.xmax,-ragdollConstraint.zmin,ragdollConstraint.ymax);

				std::stringstream ssAngLimitL;
				ssAngLimitL<<angLimitL.p<<" "<<angLimitL.y<<" "<<angLimitL.r;

				std::stringstream ssAngLimitU;
				ssAngLimitU<<angLimitU.p<<" "<<angLimitU.y<<" "<<angLimitU.r;

				joint.args = {
					{"ang_limit_l",ssAngLimitL.str()},
					{"ang_limit_u",ssAngLimitU.str()}
				};

				// Deprecated
				/*auto &joint = mdl.AddJoint(JOINT_TYPE_CONETWIST,ragdollConstraint.childIndex,ragdollConstraint.parentIndex);
				joint.args = {
					{"sp1l",std::to_string(-ragdollConstraint.xmax)},
					{"sp1u",std::to_string(-ragdollConstraint.xmin)},
					{"sp2l",std::to_string(-ragdollConstraint.zmax)},
					{"sp2u",std::to_string(-ragdollConstraint.zmin)},
					{"tsl",std::to_string(-ragdollConstraint.ymax)},
					{"tsu",std::to_string(-ragdollConstraint.ymin)},
					//{"sftn",""},
					//{"bias",""},
					//{"rlx",""}
				};*/
			}
		}
	}
	//

	// Add LODs
	/*if(bodyPart != nullptr)
	{
		for(auto lod=decltype(bodyPart->lodMeshes.size()){1};lod<bodyPart->lodMeshes.size();++lod)
		{
			auto &bodyPartLod = bodyPart->lodMeshes[lod];
			auto meshGroup = mdl.AddMeshGroup(std::string("lod") +std::to_string(lod));
			auto lodMesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
			auto lodSubMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
			lodSubMesh->SetShared(*subMesh,ModelSubMesh::ShareMode::All & ~ModelSubMesh::ShareMode::Triangles);
			lodSubMesh->GetTriangles() = bodyPartLod.indices;
			lodSubMesh->SetTexture(0);
			lodMesh->AddSubMesh(lodSubMesh);
			meshGroup->AddMesh(lodMesh);

			std::unordered_map<uint32_t,uint32_t> meshIds = {};
			meshIds[0] = lod;
			mdl.AddLODInfo(lod,meshIds);
			lodSubMesh->Update();
			lodMesh->Update();
		}
	}*/

	// Add root bone
	/*auto rootBone = std::make_shared<Bone>();
	rootBone->name = "root";
	rootBone->ID = 0u;

	for(auto it=skelBones.begin();it!=skelBones.end();++it)
	{
		(*it)->ID += 1u;
		if((*it)->parent.expired())
		{
			(*it)->parent = rootBone;
			rootBone->children.insert(std::make_pair((*it)->ID,*it));
		}
		auto oldChildren = (*it)->children;
		(*it)->children.clear();
		for(auto &pair : oldChildren)
			(*it)->children.insert(std::make_pair(pair.first +1u,pair.second));
	}
	skelBones.insert(skelBones.begin(),rootBone);

	const auto fAddRootBoneToFrame = [](Frame &frame) {
		auto &transforms = frame.GetBoneTransforms();
		auto &scales = frame.GetBoneScales();
		transforms.insert(transforms.begin(),OrientedPoint{Vector3{},uquat::identity()});
		if(scales.empty() == false)
			scales.insert(scales.begin(),Vector3{1.f,1.f,1.f});
	};

	const auto fAddRootBoneToAnim = [&fAddRootBoneToFrame](Animation &anim) {
		auto &boneList = const_cast<std::vector<uint32_t>&>(anim.GetBoneList());
		for(auto &id : boneList)
			++id;
		boneList.insert(boneList.begin(),0u);
		for(auto &frame : anim.GetFrames())
			fAddRootBoneToFrame(*frame);
	};

	for(auto &anim : mdl.GetAnimations())
		fAddRootBoneToAnim(*anim);
	rootBones = {
		{0u,rootBone}
	};

	for(auto &meshGroup : mdl.GetMeshGroups())
	{
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto &vertWeights = subMesh->GetVertexWeights();
				for(auto &vw : vertWeights)
				{
					for(auto i=0u;i<4u;++i)
					{
						auto &id = vw.boneIds[i];
						if(id == -1)
							continue;
						++id;
					}
				}
			}
		}
	}

	for(auto &att : mdl.GetAttachments())
		++att.bone;
	for(auto &colMesh : mdl.GetCollisionMeshes())
	{
		auto boneId = colMesh->GetBoneParent();
		if(boneId != -1)
			colMesh->SetBoneParent(boneId +1u);
	}
	auto &hitboxes = mdl.GetHitboxes();
	auto oldHitboxes = hitboxes;
	hitboxes.clear();
	for(auto &pair : oldHitboxes)
		hitboxes.insert(std::make_pair(pair.first +1u,pair.second));*/
	//

	auto &baseMeshes = mdl.GetBaseMeshes();
	baseMeshes.clear();
	//baseMeshes = bgBaseMeshGroups;
	baseMeshes.push_back(0);
	//

	// Generate reference
	auto refPose = std::make_shared<Frame>(*frameReference);
	for(auto i=decltype(bones.size()){0};i<bones.size();++i)
	{
		auto &pos = *refPose->GetBonePosition(i);
		auto &rot = *refPose->GetBoneOrientation(i);

		auto m = glm::toMat4(rot);
		m = glm::translate(m,pos);
		mdl.SetBindPoseBoneMatrix(i,glm::inverse(m));
	}
	reference->Localize(skeleton);
	mdl.SetReference(refPose);
	//

	// Build bind pose
	for(auto i=decltype(header.numbones){0};i<header.numbones;++i)
	{
		auto &pos = *refPose->GetBonePosition(i);
		auto &rot = *refPose->GetBoneOrientation(i);

		auto m = glm::toMat4(rot);
		m = glm::translate(m,pos);
		mdl.SetBindPoseBoneMatrix(i,glm::inverse(m));
	}
	//

	// Attachments
	for(auto &att : mdlInfo.attachments)
	{
		auto &t = att->GetTransform();
		auto tmpAng = mdl::util::convert_rotation_matrix_to_degrees(t[0][0],t[1][0],t[2][0],t[0][1],t[1][1],t[2][1],t[2][2]);

		// Transform to source system
		auto ang = EulerAngles(
			tmpAng.y,
			-tmpAng.r,
			-tmpAng.p
		);

		// Transform to pragma system; These transformations are strange, but they seem to work for all models
		ang.p = -ang.p;
		ang.y = -ang.y;
		auto rot = uquat::create(EulerAngles(0,90,0)) *uquat::create(ang);
		//auto matAng = mdl::util::euler_angles_to_matrix(ang);
		//auto rot = mdl::util::get_rotation(matAng);
		
		Vector3 pos(t[0][3],t[1][3],t[2][3]);
		umath::swap(pos.y,pos.z);
		umath::negate(pos.y);

		mdl.AddAttachment(att->GetName(),att->GetBone(),pos,rot);
	}
	//

	// Hitboxes
	for(auto &hbSet : mdlInfo.hitboxSets)
	{
		for(auto &hb : hbSet.GetHitboxes())
		{
			auto hg = (hb.groupId != 10) ? static_cast<HitGroup>(hb.groupId) : HitGroup::Gear;
			mdl.AddHitbox(hb.boneId,hg,hb.boundingBox.first,hb.boundingBox.second);
		}
	}
	//

	// Skins
	auto bFirst = true;
	for(auto &skinFamily : mdlInfo.skinFamilies)
	{
		if(bFirst == true) // Skip first skin family
		{
			bFirst = false;
			continue;
		}
		auto *texGroup = mdl.CreateTextureGroup();
		texGroup->textures.reserve(skinFamily.size());
		for(auto idx : skinFamily)
		{
			auto texId = textureTranslations.at(idx);
			texGroup->textures.push_back(texId);
		}
	}
	//

	mdl.Update(ModelUpdateFlags::All);

	return ptrMdl;
}

#pragma optimize("",on)
