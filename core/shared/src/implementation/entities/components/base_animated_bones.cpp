// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_animated;

using namespace pragma;

static void get_local_bone_position(const std::vector<math::ScaledTransform> &transforms, const animation::Bone &bone, const Vector3 &fscale = {1.f, 1.f, 1.f}, Vector3 *pos = nullptr, Quat *rot = nullptr, Vector3 *scale = nullptr)
{
	std::function<void(const animation::Bone &, Vector3 *, Quat *, Vector3 *)> apply;
	apply = [&transforms, &apply, fscale](const animation::Bone &bone, Vector3 *pos, Quat *rot, Vector3 *scale) {
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
const std::vector<math::ScaledTransform> &BaseAnimatedComponent::GetBoneTransforms() const { return const_cast<BaseAnimatedComponent &>(*this).GetBoneTransforms(); }
std::vector<math::ScaledTransform> &BaseAnimatedComponent::GetBoneTransforms() { return m_bones; }
const std::vector<math::ScaledTransform> &BaseAnimatedComponent::GetProcessedBoneTransforms() const { return const_cast<BaseAnimatedComponent &>(*this).GetProcessedBoneTransforms(); }
std::vector<math::ScaledTransform> &BaseAnimatedComponent::GetProcessedBoneTransforms() { return m_processedBones; }

bool BaseAnimatedComponent::GetBonePose(animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale, math::CoordinateSpace space) const
{
	if(boneId >= m_bones.size())
		return false;
	switch(space) {
	case math::CoordinateSpace::Local:
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
	case math::CoordinateSpace::Object:
		{
			auto &mdl = GetEntity().GetModel();
			if(!mdl)
				return false;
			auto &skeleton = mdl->GetSkeleton();
			auto bone = skeleton.GetBone(boneId).lock();
			if(bone == nullptr)
				return false;
			math::ScaledTransform t {};
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
	case math::CoordinateSpace::World:
		{
			if(!GetBonePose(boneId, optOutPos, optOutRot, optOutScale, math::CoordinateSpace::Object))
				return false;
			auto &entPose = GetEntity().GetPose();
			auto worldPose = entPose * math::ScaledTransform {optOutPos ? *optOutPos : uvec::PRM_ORIGIN, optOutRot ? *optOutRot : uquat::identity(), optOutScale ? *optOutScale : uvec::IDENTITY_SCALE};
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
bool BaseAnimatedComponent::SetBonePose(animation::BoneId boneId, const Vector3 *optPos, const Quat *optRot, const Vector3 *optScale, math::CoordinateSpace space)
{
	if(boneId >= m_bones.size())
		return false;
	switch(space) {
	case math::CoordinateSpace::Local:
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
	case math::CoordinateSpace::Object:
		{
			auto &mdl = GetEntity().GetModel();
			if(!mdl)
				return false;
			auto &skeleton = mdl->GetSkeleton();
			auto bone = skeleton.GetBone(boneId).lock();
			if(bone == nullptr)
				return false;
			math::ScaledTransform t {};
			auto parent = bone->parent.lock();
			while(parent) {
				auto &boneTransform = m_bones.at(parent->ID);
				t = boneTransform * t;
				parent = parent->parent.lock();
			}

			math::ScaledTransform relPose {optPos ? *optPos : uvec::PRM_ORIGIN, optRot ? *optRot : uquat::identity(), optScale ? *optScale : uvec::IDENTITY_SCALE};
			relPose = t.GetInverse() * relPose;
			return SetBonePose(boneId, optPos ? &relPose.GetOrigin() : nullptr, optRot ? &relPose.GetRotation() : nullptr, optScale ? &relPose.GetScale() : nullptr, math::CoordinateSpace::Local);
		}
	case math::CoordinateSpace::World:
		{
			math::ScaledTransform relPose {optPos ? *optPos : uvec::PRM_ORIGIN, optRot ? *optRot : uquat::identity(), optScale ? *optScale : uvec::IDENTITY_SCALE};
			relPose = GetEntity().GetPose().GetInverse() * relPose;
			return SetBonePose(boneId, optPos ? &relPose.GetOrigin() : nullptr, optRot ? &relPose.GetRotation() : nullptr, optScale ? &relPose.GetScale() : nullptr, math::CoordinateSpace::Object);
		}
	default:
		return false;
	}
	math::set_flag(m_stateFlags, StateFlags::AbsolutePosesDirty);

	CEOnBoneTransformChanged evData {boneId, optPos, optRot, optScale};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED, evData);
	return true;
}
bool BaseAnimatedComponent::GetReferenceBonePose(animation::BoneId boneId, math::Transform &outPose, math::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePose(boneId, outPose, space);
}
bool BaseAnimatedComponent::GetReferenceBonePose(animation::BoneId boneId, math::ScaledTransform &outPose, math::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePose(boneId, outPose, space);
}
bool BaseAnimatedComponent::GetReferenceBonePos(animation::BoneId boneId, Vector3 &outPos, math::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePos(boneId, outPos, space);
}
bool BaseAnimatedComponent::GetReferenceBoneRot(animation::BoneId boneId, Quat &outRot, math::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBoneRot(boneId, outRot, space);
}
bool BaseAnimatedComponent::GetReferenceBoneScale(animation::BoneId boneId, Vector3 &outScale, math::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBoneScale(boneId, outScale, space);
}
bool BaseAnimatedComponent::GetReferenceBonePose(animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale, math::CoordinateSpace space) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	return mdl->GetReferenceBonePose(boneId, optOutPos, optOutRot, optOutScale, space);
}

bool BaseAnimatedComponent::GetBonePose(animation::BoneId boneId, math::Transform &outPose, math::CoordinateSpace space) const { return GetBonePose(boneId, &outPose.GetOrigin(), &outPose.GetRotation(), nullptr, space); }
bool BaseAnimatedComponent::GetBonePose(animation::BoneId boneId, math::ScaledTransform &outPose, math::CoordinateSpace space) const { return GetBonePose(boneId, &outPose.GetOrigin(), &outPose.GetRotation(), &outPose.GetScale(), space); }
bool BaseAnimatedComponent::GetBonePos(animation::BoneId boneId, Vector3 &outPos, math::CoordinateSpace space) const { return GetBonePose(boneId, &outPos, nullptr, nullptr, space); }
bool BaseAnimatedComponent::GetBoneRot(animation::BoneId boneId, Quat &outRot, math::CoordinateSpace space) const { return GetBonePose(boneId, nullptr, &outRot, nullptr, space); }
bool BaseAnimatedComponent::GetBoneScale(animation::BoneId boneId, Vector3 &outScale, math::CoordinateSpace space) const { return GetBonePose(boneId, nullptr, nullptr, &outScale, space); }

bool BaseAnimatedComponent::SetBonePose(animation::BoneId boneId, const math::Transform &pose, math::CoordinateSpace space) { return SetBonePose(boneId, &pose.GetOrigin(), &pose.GetRotation(), nullptr, space); }
bool BaseAnimatedComponent::SetBonePose(animation::BoneId boneId, const math::ScaledTransform &pose, math::CoordinateSpace space) { return SetBonePose(boneId, &pose.GetOrigin(), &pose.GetRotation(), &pose.GetScale(), space); }
bool BaseAnimatedComponent::SetBonePos(animation::BoneId boneId, const Vector3 &pos, math::CoordinateSpace space) { return SetBonePose(boneId, &pos, nullptr, nullptr, space); }
bool BaseAnimatedComponent::SetBoneRot(animation::BoneId boneId, const Quat &rot, math::CoordinateSpace space) { return SetBonePose(boneId, nullptr, &rot, nullptr, space); }
bool BaseAnimatedComponent::SetBoneScale(animation::BoneId boneId, const Vector3 &scale, math::CoordinateSpace space) { return SetBonePose(boneId, nullptr, nullptr, &scale, space); }

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
bool BaseAnimatedComponent::MetaBonePoseToSkeletal(animation::MetaRigBoneType boneType, math::ScaledTransform &pose) const
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	auto &metaRig = hModel->GetMetaRig();
	if(!metaRig)
		return false;
	pose.SetRotation(pose.GetRotation() * uquat::get_inverse(metaRig->bones[math::to_integral(boneType)].normalizedRotationOffset));
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
	rot = rot * uquat::get_inverse(metaRig->bones[math::to_integral(boneType)].normalizedRotationOffset);
	return true;
}
bool BaseAnimatedComponent::SetMetaBonePose(animation::MetaRigBoneType boneType, const math::ScaledTransform &pose, math::CoordinateSpace space)
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
	math::ScaledTransform newPose;
	switch(space) {
	case math::CoordinateSpace::World:
		{
			newPose = GetEntity().GetPose().GetInverse() * pose;
			break;
		}
	case math::CoordinateSpace::Object:
		{
			newPose = pose;
			break;
		}
	case math::CoordinateSpace::Local:
		{
			SetBonePose(*boneId, pose, space);
			return true;
		}
	}
	newPose.SetRotation(newPose.GetRotation() * uquat::get_inverse(metaRig->bones[math::to_integral(boneType)].normalizedRotationOffset));
	SetBonePose(*boneId, newPose, math::CoordinateSpace::Object);
	return true;
}
bool BaseAnimatedComponent::SetMetaBonePose(animation::MetaRigBoneType boneType, const Vector3 *optPos, const Quat *optRot, const Vector3 *optScale, math::CoordinateSpace space)
{
	math::ScaledTransform curPose;
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
bool BaseAnimatedComponent::GetMetaBonePose(animation::MetaRigBoneType boneType, math::ScaledTransform &outPose, math::CoordinateSpace space) const
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
	if(!GetBonePose(*boneId, outPose, math::CoordinateSpace::Object))
		return false;
	outPose.SetRotation(outPose.GetRotation() * metaRig->bones[math::to_integral(boneType)].normalizedRotationOffset);
	switch(space) {
	case math::CoordinateSpace::World:
		{
			outPose = GetEntity().GetPose() * outPose;
			break;
		}
	case math::CoordinateSpace::Object:
		break;
	case math::CoordinateSpace::Local:
		{
			GetBonePose(*boneId, outPose, math::CoordinateSpace::Local);
			outPose.SetRotation(metaRig->bones[math::to_integral(boneType)].normalizedRotationOffset * outPose.GetRotation());
			break;
		}
	}
	return true;
}
bool BaseAnimatedComponent::GetMetaBonePose(animation::MetaRigBoneType boneType, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale, math::CoordinateSpace space) const
{
	math::ScaledTransform curPose;
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
	return InvokeEventCallbacks(baseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES, evData) == util::EventReply::Handled && evData.shouldUpdate;
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

void BaseAnimatedComponent::TransformBoneFrames(std::vector<math::Transform> &bonePoses, std::vector<Vector3> *boneScales, animation::Animation &anim, Frame *frameBlend, bool bAdd)
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
						boneScale[i] = math::lerp(1.0, boneScale[i], weight);
					boneScales->at(i) *= boneScale;
				}
				else
					boneScales->at(i) = uvec::lerp(boneScales->at(i), *scale, weight);
			}
		}
	}
}
void BaseAnimatedComponent::TransformBoneFrames(std::vector<math::Transform> &tgt, std::vector<Vector3> *boneScales, const std::shared_ptr<animation::Animation> &baseAnim, const std::shared_ptr<animation::Animation> &anim, std::vector<math::Transform> &add,
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
					boneScale[i] = math::lerp(1.0, boneScale[i], weight);
				boneScales->at(i) *= boneScale;
			}
			else
				boneScales->at(i) = uvec::lerp(boneScales->at(i), addScales->at(animBoneIdx), weight);
		}
	}
}
void BaseAnimatedComponent::BlendBonePoses(const std::vector<math::Transform> &srcBonePoses, const std::vector<Vector3> *optSrcBoneScales, const std::vector<math::Transform> &dstBonePoses, const std::vector<Vector3> *optDstBoneScales, std::vector<math::Transform> &outBonePoses,
  std::vector<Vector3> *optOutBoneScales, animation::Animation &anim, float interpFactor) const
{
	auto numBones = math::min(srcBonePoses.size(), dstBonePoses.size(), outBonePoses.size());
	auto numScales = (optSrcBoneScales && optDstBoneScales && optOutBoneScales) ? math::min(optSrcBoneScales->size(), optDstBoneScales->size(), optOutBoneScales->size(), numBones) : 0;
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
void BaseAnimatedComponent::BlendBoneFrames(std::vector<math::Transform> &tgt, std::vector<Vector3> *tgtScales, std::vector<math::Transform> &add, std::vector<Vector3> *addScales, float blendScale) const
{
	if(blendScale == 0.f)
		return;
	for(unsigned int i = 0; i < math::min(tgt.size(), add.size()); i++) {
		auto &pose = tgt.at(i);
		pose.Interpolate(add.at(i), blendScale);
		if(tgtScales != nullptr && addScales != nullptr)
			tgtScales->at(i) = uvec::lerp(tgtScales->at(i), addScales->at(i), blendScale);
	}
}

static void get_global_bone_transforms(std::vector<math::ScaledTransform> &transforms, std::unordered_map<animation::BoneId, std::shared_ptr<animation::Bone>> &childBones, const math::ScaledTransform &tParent = {})
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
void BaseAnimatedComponent::SetSkeletonUpdateListenerEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::SkeletonUpdateListenerEnabled); }
bool BaseAnimatedComponent::IsSkeletonUpdateListenerEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::SkeletonUpdateListenerEnabled); }
bool BaseAnimatedComponent::UpdateBonePoses()
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	math::set_flag(m_stateFlags, StateFlags::AbsolutePosesDirty, false);
	auto &skeleton = hModel->GetSkeleton();
	m_processedBones = m_bones;
	get_global_bone_transforms(m_processedBones, skeleton.GetRootBones());
	return true;
}
bool BaseAnimatedComponent::UpdateSkeleton()
{
	if(IsSkeletonUpdateListenerEnabled()) {
		CEOnUpdateSkeleton evData {};
		if(InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_UPDATE_SKELETON, evData) == util::EventReply::Handled)
			return evData.bonePosesHaveChanged;
	}
	if(math::is_flag_set(m_stateFlags, StateFlags::AbsolutePosesDirty) == false)
		return false;
	return UpdateBonePoses();
}

void BaseAnimatedComponent::PostAnimationsUpdated() { InvokeEventCallbacks(baseAnimatedComponent::EVENT_POST_ANIMATION_UPDATE); }

void BaseAnimatedComponent::SetPostAnimationUpdateEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::NeedsPostAnimationUpdate, enabled); }
bool BaseAnimatedComponent::IsPostAnimationUpdateEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::NeedsPostAnimationUpdate); }
