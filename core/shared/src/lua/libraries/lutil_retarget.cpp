/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_flex_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/model/model.h"
#include "pragma/lua/libraries/lutil.hpp"
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>

std::shared_ptr<Lua::util::retarget::RetargetFlexData> Lua::util::retarget::initialize_retarget_flex_data(luabind::object remapData)
{
	auto *l = remapData.interpreter();
	auto data = std::make_shared<RetargetFlexData>();
	for(luabind::iterator it {remapData}, end; it != end; ++it) {
		auto idx0 = luabind::object_cast<uint32_t>(it.key());
		luabind::object val {*it};

		for(luabind::iterator it {val}, end; it != end; ++it) {
			auto idx1 = luabind::object_cast<uint32_t>(it.key());
			luabind::object t = *it;
			RetargetFlexData::RemapData remapData {};
			remapData.minSource = luabind::object_cast<float>(t["min_source"]);
			remapData.maxSource = luabind::object_cast<float>(t["max_source"]);
			remapData.minTarget = luabind::object_cast<float>(t["min_target"]);
			remapData.maxTarget = luabind::object_cast<float>(t["max_target"]);
			data->remapData[idx0][idx1] = remapData;
		}
	}
	return data;
}
void Lua::util::retarget::retarget_flex_controllers(RetargetFlexData &retargetFlexData, pragma::BaseFlexComponent &flexCSrc, pragma::BaseFlexComponent &flexCDst)
{
	std::unordered_map<uint32_t, float> accMap;
	for(auto &pair : retargetFlexData.remapData) {
		auto val = flexCSrc.GetFlexController(pair.first);
		auto &mappingData = pair.second;
		for(auto &pair : mappingData) {
			auto flexIdDst = pair.first;
			auto &data = pair.second;
			auto srcVal = umath::clamp(val, data.minSource, data.maxSource);
			auto f = srcVal / (data.maxSource - data.minSource);
			auto dstVal = data.minTarget + f * (data.maxTarget - data.minTarget);
			auto it = accMap.find(flexIdDst);
			dstVal = dstVal + ((it != accMap.end()) ? it->second : 0.f);
			flexCDst.SetFlexController(flexIdDst, dstVal, 0.f, false);
			accMap[flexIdDst] = dstVal;
		}
	}
}

std::shared_ptr<Lua::util::retarget::RetargetData> Lua::util::retarget::initialize_retarget_data(luabind::object absBonePoses, luabind::object origBindPoseToRetargetBindPose, luabind::object origBindPoseBoneDistances,

  luabind::object bindPosesOther, luabind::object origBindPoses, luabind::object tmpPoses, luabind::object retargetPoses, luabind::object invRootPose,

  luabind::object bindPoseTransforms, luabind::object relBindPoses,

  luabind::object tUntranslatedBones, luabind::object tTranslationTable)
{
	auto retargetData = std::make_shared<RetargetData>();
	auto *l = absBonePoses.interpreter();
	retargetData->absBonePoses = Lua::table_to_vector<umath::ScaledTransform>(l, absBonePoses, 1);
	retargetData->origBindPoseToRetargetBindPose = Lua::table_to_vector<umath::ScaledTransform>(l, origBindPoseToRetargetBindPose, 2);
	retargetData->origBindPoseBoneDistances = Lua::table_to_vector<float>(l, origBindPoseBoneDistances, 3);

	for(luabind::iterator it {bindPosesOther}, end; it != end; ++it) {
		auto key = luabind::object_cast<BoneId>(it.key());
		luabind::object val {*it};

		if(key >= retargetData->bindPosesOther.size())
			retargetData->bindPosesOther.resize(key + 1);
		retargetData->bindPosesOther[key] = luabind::object_cast<umath::ScaledTransform>(val);
	}

	retargetData->origBindPoses = Lua::table_to_vector<umath::ScaledTransform>(l, origBindPoses, 5);
	retargetData->tmpPoses = Lua::table_to_vector<umath::ScaledTransform>(l, tmpPoses, 6);
	retargetData->retargetPoses = Lua::table_to_vector<umath::ScaledTransform>(l, retargetPoses, 7);
	retargetData->invRootPose = luabind::object_cast<umath::Transform>(invRootPose);

	retargetData->bindPoseTransforms = Lua::table_to_vector<umath::Transform>(l, bindPoseTransforms, 9);
	retargetData->relBindPoses = Lua::table_to_vector<umath::ScaledTransform>(l, relBindPoses, 10);

	retargetData->absBonePoses.resize(retargetData->origBindPoseToRetargetBindPose.size());

	for(luabind::iterator it {tUntranslatedBones}, end; it != end; ++it) {
		auto key = luabind::object_cast<BoneId>(it.key());
		luabind::object val {*it};

		retargetData->untranslatedBones.insert(key);
	}

	for(luabind::iterator it {tTranslationTable}, end; it != end; ++it) {
		auto key = luabind::object_cast<BoneId>(it.key());
		luabind::object val {*it};

		retargetData->translationTable[key] = TranslationData {luabind::object_cast<BoneId>(val[1]), luabind::object_cast<umath::Transform>(val[2])};
	}
	return retargetData;
}

static void FixProportionsAndUpdateUnmappedBonesAndApply(panima::Skeleton &skeleton, Lua::util::retarget::RetargetData &retargetData, std::optional<uint32_t> boneId, std::optional<uint32_t> parentBoneId)
{
	if(!boneId.has_value()) {
		for(auto &pair : skeleton.GetRootBones())
			FixProportionsAndUpdateUnmappedBonesAndApply(skeleton, retargetData, pair.first, {});
		return;
	}
	umath::ScaledTransform finalPose {};
	if(parentBoneId.has_value()) {
		auto it = retargetData.translationTable.find(*boneId);
		if(it == retargetData.translationTable.end()) {
			// Keep all bones that don't have a translation in the same relative pose
			// they had in the bind pose
			auto &poseParent = retargetData.retargetPoses[*parentBoneId];
			retargetData.retargetPoses[*boneId] = poseParent * retargetData.bindPoseTransforms[*boneId];
		}
		else {
			auto method = 1u;
			if(method == 1u) {
				auto &poseParent = retargetData.retargetPoses[*parentBoneId];
				auto poseWithBindOffset = poseParent * retargetData.relBindPoses[*boneId];
				auto &pose = retargetData.retargetPoses[*boneId];
				if(retargetData.untranslatedBones.find(*parentBoneId) == retargetData.untranslatedBones.end())
					pose.SetOrigin(poseWithBindOffset.GetOrigin());
			}
		}
		finalPose = retargetData.retargetPoses[*boneId];
	}
	else
		finalPose = retargetData.retargetPoses[*boneId];
	if(parentBoneId.has_value() && retargetData.translationTable.find(*parentBoneId) != retargetData.translationTable.end())
		finalPose = retargetData.translationTable[*parentBoneId].transform * finalPose;

	retargetData.absBonePoses[*boneId] = finalPose;

	for(auto &pair : skeleton.GetBone(*boneId).lock()->children)
		FixProportionsAndUpdateUnmappedBonesAndApply(skeleton, retargetData, pair.first, boneId);
}
void Lua::util::retarget::apply_retarget_rig(Lua::util::retarget::RetargetData &retargetData, Model &mdl, pragma::BaseAnimatedComponent &animSrc, pragma::BaseAnimatedComponent &animDst, panima::Skeleton &skeleton)
{
	animDst.UpdateSkeleton(); // Make sure the target entity's bone transforms have been updated

	auto numBones = skeleton.GetBoneCount();
	for(auto boneId = decltype(numBones) {0u}; boneId < numBones; ++boneId) {
		auto it = retargetData.translationTable.find(boneId);
		if(it != retargetData.translationTable.end()) {
			// Grab the animation pose from the target entity
			auto &data = it->second;
			auto boneIdOther = data.boneIdOther;
			auto &pose = animDst.GetProcessedBones()[boneIdOther];
			auto tmpPose1 = pose * retargetData.bindPosesOther[boneIdOther];

			auto curPose = retargetData.origBindPoses[boneId];
			curPose.SetRotation(tmpPose1.GetRotation() * curPose.GetRotation());
			curPose.SetOrigin(pose.GetOrigin());
			curPose.SetScale(pose.GetScale());
			retargetData.retargetPoses[boneId] = curPose;

			retargetData.tmpPoses[boneId] = retargetData.retargetPoses[boneId];
		}
	}

	FixProportionsAndUpdateUnmappedBonesAndApply(skeleton, retargetData, {}, {});
	std::function<void(const panima::Bone &, const umath::ScaledTransform &)> applyPose;
	applyPose = [&applyPose, &animSrc, &retargetData](const panima::Bone &bone, const umath::ScaledTransform &parentPose) {
		// We need to bring all bone poses into relative space (relative to the respective parent), as well as
		// apply the bind pose conversion transform.
		auto boneId = bone.ID;
		auto pose = retargetData.absBonePoses[boneId] * retargetData.origBindPoseToRetargetBindPose[boneId];
		auto relPose = parentPose.GetInverse() * pose;
		auto *srcScale = animSrc.GetBoneScale(boneId);
		if(srcScale)
			relPose.SetScale(*srcScale);
		animSrc.SetBonePosition(boneId, relPose.GetOrigin(), relPose.GetRotation(), relPose.GetScale());
		// TODO: There are currently a few issues with scaling (e.g. broken eyes), so we'll disable it for now. This should be re-enabled once the issues have been resolved!
		// UPDATE: Broken eyes should now be fixed with scaling, so it should work properly now? (TODO: TESTME and remove the line below if all is in order)
		// animSrc:SetBoneScale(boneId,Vector(1,1,1))
		for(auto &pair : bone.children)
			applyPose(*pair.second, pose);
	};
	const auto &invRootPose = retargetData.invRootPose;
	for(auto &pair : skeleton.GetRootBones())
		applyPose(*pair.second, invRootPose);

	// TODO: Remove this once new animation system is fully implemented
	for(auto &pair : animSrc.GetAnimationSlotInfos()) {
		auto slot = pair.first;
		auto &slotInfo = pair.second;
		auto animIdx = slotInfo.animation;
		auto anim = mdl.GetAnimation(animIdx);
		if(!anim)
			continue;
		auto frame = anim->GetFrame(0);
		auto n = anim->GetBoneCount();
		auto &boneIds = anim->GetBoneList();
		auto &transforms = animSrc.GetBoneTransforms();
		for(auto i = decltype(n) {0u}; i < n; ++i) {
			auto boneId = boneIds[i];
			auto &pose = transforms[boneId];
			umath::ScaledTransform framePose;
			frame->GetBonePose(i, framePose);
			pose = pose * framePose;
			animSrc.SetBonePosition(boneId, pose.GetOrigin(), pose.GetRotation(), pose.GetScale());
		}
	}
}
