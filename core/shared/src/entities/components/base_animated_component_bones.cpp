/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/model/model.h"
#include "pragma/model/animation/skeleton.hpp"
#include "pragma/model/animation/bone.hpp"
#include "pragma/model/animation/meta_rig.hpp"
using namespace pragma;
static void get_local_bone_position(const std::vector<umath::ScaledTransform> &transforms, const pragma::animation::Bone &bone, const Vector3 &fscale = {1.f, 1.f, 1.f}, Vector3 *pos = nullptr, Quat *rot = nullptr, Vector3 *scale = nullptr)
{
	std::function<void(const pragma::animation::Bone &, Vector3 *, Quat *, Vector3 *)> apply;
	apply = [&transforms, &apply, fscale](const pragma::animation::Bone &bone, Vector3 *pos, Quat *rot, Vector3 *scale) {
		auto parent = bone.parent.lock();
		if(parent != nullptr)
			apply(*parent, pos, rot, scale);
		auto &tParent = transforms[bone.ID];
		auto &posParent = tParent.GetOrigin();
		auto &rotParent = tParent.GetRotation();
		auto inv = uquat::get_inverse(rotParent);
		if(pos != nullptr) {
			*pos -= posParent * fscale;
			uvec::rotate(pos, inv);
		}
		if(rot != nullptr)
			*rot = inv * (*rot);
	};
	auto parent = bone.parent.lock();
	if(parent != nullptr)
		apply(*parent, pos, rot, scale);
}
UInt32 BaseAnimatedComponent::GetBoneCount() const { return CInt32(m_bones.size()); }
const std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetBoneTransforms() const { return const_cast<BaseAnimatedComponent &>(*this).GetBoneTransforms(); }
std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetBoneTransforms() { return m_bones; }
const std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetProcessedBoneTransforms() const { return const_cast<BaseAnimatedComponent &>(*this).GetProcessedBoneTransforms(); }
std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetProcessedBoneTransforms() { return m_processedBones; }

bool BaseAnimatedComponent::GetBonePose(animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale, umath::CoordinateSpace space) const
{
	if(boneId >= m_bones.size())
		return false;
	switch(space) {
	case umath::CoordinateSpace::Local:
		{
			auto &bonePose = m_bones[boneId];
			if(optOutPos)
				*optOutPos = bonePose.GetOrigin();
			if(optOutRot)
				*optOutRot = bonePose.GetRotation();
			if(optOutScale)
				*optOutScale = bonePose.GetScale();
			break;
		}
	case umath::CoordinateSpace::Object:
		{
			auto &mdl = GetEntity().GetModel();
			if(!mdl)
				return false;
			auto &skeleton = mdl->GetSkeleton();
			auto bone = skeleton.GetBone(boneId).lock();
			if(bone == nullptr)
				return false;
			umath::ScaledTransform t {};
			while(bone) {
				auto &boneTransform = m_bones.at(bone->ID);
				t = boneTransform * t;
				bone = bone->parent.lock();
			}
			if(optOutPos)
				*optOutPos = t.GetOrigin();
			if(optOutRot)
				*optOutRot = t.GetRotation();
			if(optOutScale)
				*optOutScale = t.GetScale();
			break;
		}
	case umath::CoordinateSpace::World:
		{
			if(!GetBonePose(boneId, optOutPos, optOutRot, optOutScale, umath::CoordinateSpace::Object))
				return false;
			auto &entPose = GetEntity().GetPose();
			auto worldPose = entPose * umath::ScaledTransform {optOutPos ? *optOutPos : uvec::ORIGIN, optOutRot ? *optOutRot : uquat::identity(), optOutScale ? *optOutScale : uvec::IDENTITY_SCALE};
			if(optOutPos)
				*optOutPos = worldPose.GetOrigin();
			if(optOutRot)
				*optOutRot = worldPose.GetRotation();
			if(optOutScale)
				*optOutScale = worldPose.GetScale();
			break;
		}
	default:
		return false;
	}
	return true;
}
bool BaseAnimatedComponent::SetBonePose(animation::BoneId boneId, const Vector3 *optPos, const Quat *optRot, const Vector3 *optScale, umath::CoordinateSpace space)
{
	if(boneId >= m_bones.size())
		return false;
	switch(space) {
	case umath::CoordinateSpace::Local:
		{
			auto &bonePose = m_bones[boneId];
			if(optPos)
				bonePose.SetOrigin(*optPos);
			if(optRot)
				bonePose.SetRotation(*optRot);
			if(optScale)
				bonePose.SetScale(*optScale);
			break;
		}
	case umath::CoordinateSpace::Object:
		{
			auto &mdl = GetEntity().GetModel();
			if(!mdl)
				return false;
			auto &skeleton = mdl->GetSkeleton();
			auto bone = skeleton.GetBone(boneId).lock();
			if(bone == nullptr)
				return false;
			umath::ScaledTransform t {};
			auto parent = bone->parent.lock();
			while(parent) {
				auto &boneTransform = m_bones.at(parent->ID);
				t = boneTransform * t;
				parent = parent->parent.lock();
			}

			umath::ScaledTransform relPose {optPos ? *optPos : uvec::ORIGIN, optRot ? *optRot : uquat::identity(), optScale ? *optScale : uvec::IDENTITY_SCALE};
			relPose = t.GetInverse() * relPose;
			return SetBonePose(boneId, optPos ? &relPose.GetOrigin() : nullptr, optRot ? &relPose.GetRotation() : nullptr, optScale ? &relPose.GetScale() : nullptr, umath::CoordinateSpace::Local);
		}
	case umath::CoordinateSpace::World:
		{
			umath::ScaledTransform relPose {optPos ? *optPos : uvec::ORIGIN, optRot ? *optRot : uquat::identity(), optScale ? *optScale : uvec::IDENTITY_SCALE};
			relPose = GetEntity().GetPose().GetInverse() * relPose;
			return SetBonePose(boneId, optPos ? &relPose.GetOrigin() : nullptr, optRot ? &relPose.GetRotation() : nullptr, optScale ? &relPose.GetScale() : nullptr, umath::CoordinateSpace::Object);
		}
	default:
		return false;
	}
	umath::set_flag(m_stateFlags, StateFlags::AbsolutePosesDirty);

	CEOnBoneTransformChanged evData {boneId, optPos, optRot, optScale};
	InvokeEventCallbacks(EVENT_ON_BONE_TRANSFORM_CHANGED, evData);
	return true;
}
bool BaseAnimatedComponent::GetReferenceBonePose(animation::BoneId boneId, umath::Transform &outPose, umath::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePose(boneId, outPose, space);
}
bool BaseAnimatedComponent::GetReferenceBonePose(animation::BoneId boneId, umath::ScaledTransform &outPose, umath::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePose(boneId, outPose, space);
}
bool BaseAnimatedComponent::GetReferenceBonePos(animation::BoneId boneId, Vector3 &outPos, umath::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePos(boneId, outPos, space);
}
bool BaseAnimatedComponent::GetReferenceBoneRot(animation::BoneId boneId, Quat &outRot, umath::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBoneRot(boneId, outRot, space);
}
bool BaseAnimatedComponent::GetReferenceBoneScale(animation::BoneId boneId, Vector3 &outScale, umath::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBoneScale(boneId, outScale, space);
}
bool BaseAnimatedComponent::GetReferenceBonePose(animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale, umath::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePose(boneId, optOutPos, optOutRot, optOutScale, space);
}

bool BaseAnimatedComponent::GetBonePose(animation::BoneId boneId, umath::Transform &outPose, umath::CoordinateSpace space) const { return GetBonePose(boneId, &outPose.GetOrigin(), &outPose.GetRotation(), nullptr, space); }
bool BaseAnimatedComponent::GetBonePose(animation::BoneId boneId, umath::ScaledTransform &outPose, umath::CoordinateSpace space) const { return GetBonePose(boneId, &outPose.GetOrigin(), &outPose.GetRotation(), &outPose.GetScale(), space); }
bool BaseAnimatedComponent::GetBonePos(animation::BoneId boneId, Vector3 &outPos, umath::CoordinateSpace space) const { return GetBonePose(boneId, &outPos, nullptr, nullptr, space); }
bool BaseAnimatedComponent::GetBoneRot(animation::BoneId boneId, Quat &outRot, umath::CoordinateSpace space) const { return GetBonePose(boneId, nullptr, &outRot, nullptr, space); }
bool BaseAnimatedComponent::GetBoneScale(animation::BoneId boneId, Vector3 &outScale, umath::CoordinateSpace space) const { return GetBonePose(boneId, nullptr, nullptr, &outScale, space); }

bool BaseAnimatedComponent::SetBonePose(animation::BoneId boneId, const umath::Transform &pose, umath::CoordinateSpace space) { return SetBonePose(boneId, &pose.GetOrigin(), &pose.GetRotation(), nullptr, space); }
bool BaseAnimatedComponent::SetBonePose(animation::BoneId boneId, const umath::ScaledTransform &pose, umath::CoordinateSpace space) { return SetBonePose(boneId, &pose.GetOrigin(), &pose.GetRotation(), &pose.GetScale(), space); }
bool BaseAnimatedComponent::SetBonePos(animation::BoneId boneId, const Vector3 &pos, umath::CoordinateSpace space) { return SetBonePose(boneId, &pos, nullptr, nullptr, space); }
bool BaseAnimatedComponent::SetBoneRot(animation::BoneId boneId, const Quat &rot, umath::CoordinateSpace space) { return SetBonePose(boneId, nullptr, &rot, nullptr, space); }
bool BaseAnimatedComponent::SetBoneScale(animation::BoneId boneId, const Vector3 &scale, umath::CoordinateSpace space) { return SetBonePose(boneId, nullptr, nullptr, &scale, space); }

std::optional<animation::BoneId> BaseAnimatedComponent::GetMetaBoneId(animation::MetaRigBoneType boneType) const
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return {};
	auto &metaRig = hModel->GetMetaRig();
	if(!metaRig)
		return {};
	auto boneId = metaRig->GetBoneId(animation::get_meta_rig_bone_type_name(boneType));
	return (boneId != animation::INVALID_BONE_INDEX) ? boneId : std::optional<animation::BoneId> {};
}
bool BaseAnimatedComponent::MetaBonePoseToSkeletal(animation::MetaRigBoneType boneType, umath::ScaledTransform &pose) const
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	auto &metaRig = hModel->GetMetaRig();
	if(!metaRig)
		return false;
	pose.SetRotation(pose.GetRotation() * uquat::get_inverse(metaRig->bones[umath::to_integral(boneType)].normalizedRotationOffset));
	return true;
}
bool BaseAnimatedComponent::MetaBoneRotationToSkeletal(animation::MetaRigBoneType boneType, Quat &rot) const
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	auto &metaRig = hModel->GetMetaRig();
	if(!metaRig)
		return false;
	rot = rot * uquat::get_inverse(metaRig->bones[umath::to_integral(boneType)].normalizedRotationOffset);
	return true;
}
bool BaseAnimatedComponent::SetMetaBonePose(animation::MetaRigBoneType boneType, const umath::ScaledTransform &pose, umath::CoordinateSpace space)
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	auto &metaRig = hModel->GetMetaRig();
	if(!metaRig)
		return false;
	auto boneId = GetMetaBoneId(boneType);
	if(!boneId)
		return false;
	umath::ScaledTransform newPose;
	switch(space) {
	case umath::CoordinateSpace::World:
		{
			newPose = GetEntity().GetPose().GetInverse() * pose;
			break;
		}
	case umath::CoordinateSpace::Object:
		{
			newPose = pose;
			break;
		}
	case umath::CoordinateSpace::Local:
		{
			SetBonePose(*boneId, pose, space);
			return true;
		}
	}
	newPose.SetRotation(newPose.GetRotation() * uquat::get_inverse(metaRig->bones[umath::to_integral(boneType)].normalizedRotationOffset));
	SetBonePose(*boneId, newPose, umath::CoordinateSpace::Object);
	return true;
}
bool BaseAnimatedComponent::SetMetaBonePose(animation::MetaRigBoneType boneType, const Vector3 *optPos, const Quat *optRot, const Vector3 *optScale, umath::CoordinateSpace space)
{
	umath::ScaledTransform curPose;
	if(!GetMetaBonePose(boneType, curPose, space))
		return false;
	if(optPos)
		curPose.SetOrigin(*optPos);
	if(optRot)
		curPose.SetRotation(*optRot);
	if(optScale)
		curPose.SetScale(*optScale);
	return SetMetaBonePose(boneType, curPose, space);
}
bool BaseAnimatedComponent::GetMetaBonePose(animation::MetaRigBoneType boneType, umath::ScaledTransform &outPose, umath::CoordinateSpace space) const
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return {};
	auto &metaRig = hModel->GetMetaRig();
	if(!metaRig)
		return {};
	auto boneId = GetMetaBoneId(boneType);
	if(!boneId)
		return false;
	if(!GetBonePose(*boneId, outPose, umath::CoordinateSpace::Object))
		return false;
	outPose.SetRotation(outPose.GetRotation() * metaRig->bones[umath::to_integral(boneType)].normalizedRotationOffset);
	switch(space) {
	case umath::CoordinateSpace::World:
		{
			outPose = GetEntity().GetPose() * outPose;
			break;
		}
	case umath::CoordinateSpace::Object:
		break;
	case umath::CoordinateSpace::Local:
		{
			GetBonePose(*boneId, outPose, umath::CoordinateSpace::Local);
			outPose.SetRotation(metaRig->bones[umath::to_integral(boneType)].normalizedRotationOffset * outPose.GetRotation());
			break;
		}
	}
	return true;
}
bool BaseAnimatedComponent::GetMetaBonePose(animation::MetaRigBoneType boneType, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale, umath::CoordinateSpace space) const
{
	umath::ScaledTransform curPose;
	if(!GetMetaBonePose(boneType, curPose, space))
		return false;
	if(optOutPos)
		*optOutPos = curPose.GetOrigin();
	if(optOutRot)
		*optOutRot = curPose.GetRotation();
	if(optOutScale)
		*optOutScale = curPose.GetScale();
	return true;
}

std::optional<Mat4> BaseAnimatedComponent::GetBoneMatrix(unsigned int boneID) const
{
	if(boneID >= m_bones.size())
		return {};
	return m_bones[boneID].ToMatrix();
}
bool BaseAnimatedComponent::ShouldUpdateBones() const
{
	if(IsPlayingAnimation())
		return true;
	CEShouldUpdateBones evData {};
	return InvokeEventCallbacks(EVENT_SHOULD_UPDATE_BONES, evData) == util::EventReply::Handled && evData.shouldUpdate;
}

FPlayAnim BaseAnimatedComponent::GetBaseAnimationFlags() const { return m_baseAnim.flags; }
void BaseAnimatedComponent::SetBaseAnimationFlags(FPlayAnim flags) { m_baseAnim.flags = flags; }

std::optional<FPlayAnim> BaseAnimatedComponent::GetLayeredAnimationFlags(uint32_t layerIdx) const
{
	auto it = m_animSlots.find(layerIdx);
	if(it == m_animSlots.end())
		return {};
	return it->second.flags;
}
void BaseAnimatedComponent::SetLayeredAnimationFlags(uint32_t layerIdx, FPlayAnim flags)
{
	auto it = m_animSlots.find(layerIdx);
	if(it == m_animSlots.end())
		return;
	it->second.flags = flags;
}

void BaseAnimatedComponent::TransformBoneFrames(std::vector<umath::Transform> &bonePoses, std::vector<Vector3> *boneScales, pragma::animation::Animation &anim, Frame *frameBlend, bool bAdd)
{
	for(unsigned int i = 0; i < bonePoses.size(); i++) {
		auto &pose = bonePoses[i];
		auto *poseFrame = frameBlend->GetBoneTransform(i);
		auto weight = anim.GetBoneWeight(i);
		if(poseFrame) {
			if(bAdd == true)
				pose *= *poseFrame * weight;
			else
				pose.Interpolate(*poseFrame, weight);
		}
		if(boneScales != nullptr) {
			auto *scale = frameBlend->GetBoneScale(i);
			if(scale != nullptr) {
				if(bAdd == true) {
					auto boneScale = *scale;
					for(uint8_t i = 0; i < 3; ++i)
						boneScale[i] = umath::lerp(1.0, boneScale[i], weight);
					boneScales->at(i) *= boneScale;
				}
				else
					boneScales->at(i) = uvec::lerp(boneScales->at(i), *scale, weight);
			}
		}
	}
}
void BaseAnimatedComponent::TransformBoneFrames(std::vector<umath::Transform> &tgt, std::vector<Vector3> *boneScales, const std::shared_ptr<pragma::animation::Animation> &baseAnim, const std::shared_ptr<pragma::animation::Animation> &anim, std::vector<umath::Transform> &add,
  std::vector<Vector3> *addScales, bool bAdd)
{
	for(auto i = decltype(tgt.size()) {0}; i < tgt.size(); ++i) {
		auto boneId = baseAnim->GetBoneList()[i];
		auto animBoneIdx = anim->LookupBone(boneId);
		if(animBoneIdx == -1 || animBoneIdx >= add.size())
			continue;
		auto &pose = tgt.at(i);
		auto weight = anim->GetBoneWeight(animBoneIdx);
		if(bAdd == true)
			pose *= add.at(animBoneIdx) * weight;
		else
			pose.Interpolate(add.at(animBoneIdx), weight);
		if(boneScales != nullptr && addScales != nullptr) {
			if(bAdd == true) {
				auto boneScale = addScales->at(animBoneIdx);
				for(uint8_t i = 0; i < 3; ++i)
					boneScale[i] = umath::lerp(1.0, boneScale[i], weight);
				boneScales->at(i) *= boneScale;
			}
			else
				boneScales->at(i) = uvec::lerp(boneScales->at(i), addScales->at(animBoneIdx), weight);
		}
	}
}
void BaseAnimatedComponent::BlendBonePoses(const std::vector<umath::Transform> &srcBonePoses, const std::vector<Vector3> *optSrcBoneScales, const std::vector<umath::Transform> &dstBonePoses, const std::vector<Vector3> *optDstBoneScales, std::vector<umath::Transform> &outBonePoses,
  std::vector<Vector3> *optOutBoneScales, pragma::animation::Animation &anim, float interpFactor) const
{
	auto numBones = umath::min(srcBonePoses.size(), dstBonePoses.size(), outBonePoses.size());
	auto numScales = (optSrcBoneScales && optDstBoneScales && optOutBoneScales) ? umath::min(optSrcBoneScales->size(), optDstBoneScales->size(), optOutBoneScales->size(), numBones) : 0;
	for(auto boneId = decltype(numBones) {0u}; boneId < numBones; ++boneId) {
		auto &srcPose = srcBonePoses.at(boneId);
		auto dstPose = dstBonePoses.at(boneId);
		auto boneWeight = anim.GetBoneWeight(boneId);
		auto boneInterpFactor = boneWeight * interpFactor;
		auto &outPose = (outBonePoses.at(boneId) = srcPose);
		outPose.Interpolate(dstPose, boneInterpFactor);

		// Scaling
		if(boneId >= numScales)
			continue;
		optOutBoneScales->at(boneId) = uvec::lerp(optSrcBoneScales->at(boneId), optDstBoneScales->at(boneId) * boneWeight, interpFactor);
	}
}
void BaseAnimatedComponent::BlendBoneFrames(std::vector<umath::Transform> &tgt, std::vector<Vector3> *tgtScales, std::vector<umath::Transform> &add, std::vector<Vector3> *addScales, float blendScale) const
{
	if(blendScale == 0.f)
		return;
	for(unsigned int i = 0; i < umath::min(tgt.size(), add.size()); i++) {
		auto &pose = tgt.at(i);
		pose.Interpolate(add.at(i), blendScale);
		if(tgtScales != nullptr && addScales != nullptr)
			tgtScales->at(i) = uvec::lerp(tgtScales->at(i), addScales->at(i), blendScale);
	}
}

static void get_global_bone_transforms(std::vector<umath::ScaledTransform> &transforms, std::unordered_map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> &childBones, const umath::ScaledTransform &tParent = {})
{
	for(auto &pair : childBones) {
		auto boneId = pair.first;
		auto &bone = pair.second;
		if(boneId >= transforms.size())
			continue;
		auto &t = transforms.at(boneId);
		t.SetOrigin(t.GetOrigin() * tParent.GetScale());
		t = tParent * t;
		get_global_bone_transforms(transforms, bone->children, t);
	}
}
void BaseAnimatedComponent::SetSkeletonUpdateListenerEnabled(bool enabled) { umath::set_flag(m_stateFlags, StateFlags::SkeletonUpdateListenerEnabled); }
bool BaseAnimatedComponent::IsSkeletonUpdateListenerEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::SkeletonUpdateListenerEnabled); }
bool BaseAnimatedComponent::UpdateBonePoses()
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	umath::set_flag(m_stateFlags, StateFlags::AbsolutePosesDirty, false);
	auto &skeleton = hModel->GetSkeleton();
	m_processedBones = m_bones;
	get_global_bone_transforms(m_processedBones, skeleton.GetRootBones());
	return true;
}
bool BaseAnimatedComponent::UpdateSkeleton()
{
	if(IsSkeletonUpdateListenerEnabled()) {
		CEOnUpdateSkeleton evData {};
		if(InvokeEventCallbacks(EVENT_ON_UPDATE_SKELETON, evData) == util::EventReply::Handled)
			return evData.bonePosesHaveChanged;
	}
	if(umath::is_flag_set(m_stateFlags, StateFlags::AbsolutePosesDirty) == false)
		return false;
	return UpdateBonePoses();
}

void BaseAnimatedComponent::PostAnimationsUpdated() { InvokeEventCallbacks(EVENT_POST_ANIMATION_UPDATE); }

void BaseAnimatedComponent::SetPostAnimationUpdateEnabled(bool enabled) { umath::set_flag(m_stateFlags, StateFlags::NeedsPostAnimationUpdate, enabled); }
bool BaseAnimatedComponent::IsPostAnimationUpdateEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::NeedsPostAnimationUpdate); }
