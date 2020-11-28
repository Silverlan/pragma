/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

static void get_local_bone_position(std::vector<umath::ScaledTransform> &transforms,std::shared_ptr<Bone> &bone,const Vector3 &fscale={1.f,1.f,1.f},Vector3 *pos=nullptr,Quat *rot=nullptr,Vector3 *scale=nullptr)
{
	std::function<void(std::shared_ptr<Bone>&,Vector3*,Quat*,Vector3*)> apply;
	apply = [&transforms,&apply,fscale](std::shared_ptr<Bone> &bone,Vector3 *pos,Quat *rot,Vector3 *scale) {
		auto parent = bone->parent.lock();
		if(parent != nullptr)
			apply(parent,pos,rot,scale);
		auto &tParent = transforms[bone->ID];
		auto &posParent = tParent.GetOrigin();
		auto &rotParent = tParent.GetRotation();
		auto inv = uquat::get_inverse(rotParent);
		if(pos != nullptr)
		{
			*pos -= posParent *fscale;
			uvec::rotate(pos,inv);
		}
		if(rot != nullptr)
			*rot = inv *(*rot);
	};
	auto parent = bone->parent.lock();
	if(parent != nullptr)
		apply(parent,pos,rot,scale);
}
static void get_local_bone_position(const std::shared_ptr<Model> &mdl,std::vector<umath::ScaledTransform> &transforms,std::shared_ptr<Bone> &bone,const Vector3 &fscale={1.f,1.f,1.f},Vector3 *pos=nullptr,Quat *rot=nullptr,Vector3 *scale=nullptr)
{
	get_local_bone_position(transforms,bone,fscale,pos,rot,scale);

	// Obsolete? Not sure what this was for
	/*if(rot == nullptr)
		return;
	auto anim = mdl->GetAnimation(0);
	if(anim != nullptr)
	{
		auto frame = anim->GetFrame(0); // Reference pose
		if(frame != nullptr)
		{
			auto *frameRot = frame->GetBoneOrientation(0); // Rotation of root bone
			if(frameRot != nullptr)
				*rot *= *frameRot;
		}
	}*/
}
UInt32 BaseAnimatedComponent::GetBoneCount() const {return CInt32(m_bones.size());}
const std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetBoneTransforms() const {return const_cast<BaseAnimatedComponent&>(*this).GetBoneTransforms();}
std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetBoneTransforms() {return m_bones;}
const std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetProcessedBoneTransforms() const {return const_cast<BaseAnimatedComponent&>(*this).GetProcessedBoneTransforms();}
std::vector<umath::ScaledTransform> &BaseAnimatedComponent::GetProcessedBoneTransforms() {return m_processedBones;}

Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 &scale) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetOrigin();
	rot = m_bones[boneId].GetRotation();
	scale = m_bones[boneId].GetScale();
	return true;
}
Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetOrigin();
	rot = m_bones[boneId].GetRotation();
	return true;
}
Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetOrigin();
	return true;
}
Bool BaseAnimatedComponent::GetBoneRotation(UInt32 boneId,Quat &rot) const
{
	if(boneId >= m_bones.size())
		return false;
	rot = m_bones[boneId].GetRotation();
	return true;
}
Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos,EulerAngles &ang) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetOrigin();
	ang = EulerAngles(m_bones[boneId].GetRotation());
	return true;
}
Bool BaseAnimatedComponent::GetBoneAngles(UInt32 boneId,EulerAngles &ang) const
{
	if(boneId >= m_bones.size())
		return false;
	ang = EulerAngles(m_bones[boneId].GetRotation());
	return true;
}
const Vector3 *BaseAnimatedComponent::GetBonePosition(UInt32 boneId) const
{
	if(boneId >= m_bones.size())
		return nullptr;
	return &m_bones[boneId].GetOrigin();
}
const Quat *BaseAnimatedComponent::GetBoneRotation(UInt32 boneId) const
{
	if(boneId >= m_bones.size())
		return nullptr;
	return &m_bones[boneId].GetRotation();
}
// See also lanimation.cpp
Bool BaseAnimatedComponent::GetLocalBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 *scale) const
{
	if(boneId >= m_bones.size())
		return false;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return false;
	umath::ScaledTransform t {};
	while(bone)
	{
		auto &boneTransform = m_bones.at(bone->ID);
		t = boneTransform *t;
		bone = bone->parent.lock();
	}
	pos = t.GetOrigin();
	rot = t.GetRotation();
	if(scale)
		*scale = t.GetScale();
	return true;
}
Bool BaseAnimatedComponent::GetLocalBonePosition(UInt32 boneId,Vector3 &pos) const
{
	Quat rot;
	if(GetLocalBonePosition(boneId,pos,rot) == false)
		return false;
	return true;
}
Bool BaseAnimatedComponent::GetLocalBoneRotation(UInt32 boneId,Quat &rot) const
{
	Vector3 pos;
	if(GetLocalBonePosition(boneId,pos,rot) == false)
		return false;
	return true;
}
Bool BaseAnimatedComponent::GetGlobalBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 *scale) const
{
	if(GetLocalBonePosition(boneId,pos,rot,scale) == false)
		return false;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(!pTrComponent)
		return true;
	uvec::local_to_world(pTrComponent->GetOrigin(),pTrComponent->GetRotation(),pos,rot);//uvec::local_to_world(GetOrigin(),GetOrientation(),pos,rot);
	return true;
}
Bool BaseAnimatedComponent::GetGlobalBonePosition(UInt32 boneId,Vector3 &pos) const
{
	if(GetLocalBonePosition(boneId,pos) == false)
		return false;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(!pTrComponent)
		return true;
	uvec::local_to_world(pTrComponent->GetOrigin(),pTrComponent->GetRotation(),pos);//uvec::local_to_world(GetOrigin(),GetOrientation(),pos);
	return true;
}
Bool BaseAnimatedComponent::GetGlobalBoneRotation(UInt32 boneId,Quat &rot) const
{
	if(GetLocalBoneRotation(boneId,rot) == false)
		return false;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(!pTrComponent)
		return true;
	uvec::local_to_world(pTrComponent->GetRotation(),rot);
	return true;
}
void BaseAnimatedComponent::SetBoneScale(uint32_t boneId,const Vector3 &scale)
{
	if(boneId >= m_bones.size())
		return;
	m_bones.at(boneId).SetScale(scale);
}
const Vector3 *BaseAnimatedComponent::GetBoneScale(uint32_t boneId) const
{
	if(boneId >= m_bones.size())
		return nullptr;
	return &m_bones.at(boneId).GetScale();
}
void BaseAnimatedComponent::SetBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 *scale,Bool updatePhysics)
{
	if(boneId >= m_bones.size())
		return;
	m_bones[boneId].SetOrigin(pos);
	m_bones[boneId].SetRotation(rot);
	if(scale != nullptr)
		m_bones[boneId].SetScale(*scale);
	//if(updatePhysics == false)
	//	return;
	CEOnBoneTransformChanged evData {boneId,&pos,&rot,scale};
	InvokeEventCallbacks(EVENT_ON_BONE_TRANSFORM_CHANGED,evData);
}
void BaseAnimatedComponent::SetBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale) {SetBonePosition(boneId,pos,rot,&scale,true);}
void BaseAnimatedComponent::SetBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot) {SetBonePosition(boneId,pos,rot,nullptr,true);}
void BaseAnimatedComponent::SetBonePosition(UInt32 boneId,const Vector3 &pos,const EulerAngles &ang) {SetBonePosition(boneId,pos,uquat::create(ang));}
void BaseAnimatedComponent::SetBonePosition(UInt32 boneId,const Vector3 &pos)
{
	if(boneId >= m_bones.size())
		return;
	m_bones[boneId].SetOrigin(pos);

	CEOnBoneTransformChanged evData {boneId,&pos,nullptr,nullptr};
	InvokeEventCallbacks(EVENT_ON_BONE_TRANSFORM_CHANGED,evData);
}
void BaseAnimatedComponent::SetBoneRotation(UInt32 boneId,const Quat &rot)
{
	if(boneId >= m_bones.size())
		return;
	m_bones[boneId].SetRotation(rot);

	CEOnBoneTransformChanged evData {boneId,nullptr,&rot,nullptr};
	InvokeEventCallbacks(EVENT_ON_BONE_TRANSFORM_CHANGED,evData);
}

void BaseAnimatedComponent::SetLocalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale)
{
	if(boneId >= m_bones.size())
		return;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	auto npos = pos;
	auto nrot = rot;
	auto nscale = scale;
	auto pTrComponent = GetEntity().GetTransformComponent();
	get_local_bone_position(hModel,m_bones,bone,pTrComponent ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},&npos,&nrot,&nscale);
	SetBonePosition(boneId,npos,nrot,nscale);
}
void BaseAnimatedComponent::SetLocalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot)
{
	if(boneId >= m_bones.size())
		return;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	auto npos = pos;
	auto nrot = rot;
	auto pTrComponent = GetEntity().GetTransformComponent();
	get_local_bone_position(hModel,m_bones,bone,pTrComponent ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},&npos,&nrot);
	SetBonePosition(boneId,npos,nrot);
}
void BaseAnimatedComponent::SetLocalBonePosition(UInt32 boneId,const Vector3 &pos)
{
	if(boneId >= m_bones.size())
		return;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	auto npos = pos;
	auto pTrComponent = GetEntity().GetTransformComponent();
	get_local_bone_position(hModel,m_bones,bone,pTrComponent ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},&npos);
	SetBonePosition(boneId,npos);
}
void BaseAnimatedComponent::SetLocalBoneRotation(UInt32 boneId,const Quat &rot)
{
	if(boneId >= m_bones.size())
		return;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	auto nrot = rot;
	auto pTrComponent = GetEntity().GetTransformComponent();
	get_local_bone_position(hModel,m_bones,bone,pTrComponent ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},nullptr,&nrot);
	SetBoneRotation(boneId,nrot);
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
	return InvokeEventCallbacks(EVENT_SHOULD_UPDATE_BONES,evData) == util::EventReply::Handled && evData.shouldUpdate;
}

void BaseAnimatedComponent::TransformBoneFrames(std::vector<umath::Transform> &bonePoses,std::vector<Vector3> *boneScales,Animation &anim,Frame *frameBlend,bool bAdd)
{
	for(unsigned int i=0;i<bonePoses.size();i++)
	{
		auto &pose = bonePoses[i];
		auto *poseFrame = frameBlend->GetBoneTransform(i);
		auto weight = anim.GetBoneWeight(i);
		if(poseFrame)
		{
			if(bAdd == true)
				pose *= *poseFrame *weight;
			else
				pose.Interpolate(*poseFrame,weight);
		}
		if(boneScales != nullptr)
		{
			auto *scale = frameBlend->GetBoneScale(i);
			if(scale != nullptr)
			{
				if(bAdd == true)
				{
					auto boneScale = *scale;
					for(uint8_t i=0;i<3;++i)
						boneScale[i] = umath::lerp(1.0,boneScale[i],weight);
					boneScales->at(i) *= boneScale;
				}
				else
					boneScales->at(i) = uvec::lerp(boneScales->at(i),*scale,weight);
			}
		}
	}
}
void BaseAnimatedComponent::TransformBoneFrames(std::vector<umath::Transform> &tgt,std::vector<Vector3> *boneScales,const std::shared_ptr<Animation> &anim,std::vector<umath::Transform> &add,std::vector<Vector3> *addScales,bool bAdd)
{
	for(auto i=decltype(tgt.size()){0};i<tgt.size();++i)
	{
		auto animBoneIdx = anim->LookupBone(i);
		if(animBoneIdx == -1 || animBoneIdx >= add.size())
			continue;
		auto &pose = tgt.at(i);
		auto weight = anim->GetBoneWeight(i);
		if(bAdd == true)
			pose *= add.at(animBoneIdx) *weight;
		else
			pose.Interpolate(add.at(animBoneIdx),weight);
		if(boneScales != nullptr && addScales != nullptr)
		{
			if(bAdd == true)
			{
				auto boneScale = addScales->at(animBoneIdx);
				for(uint8_t i=0;i<3;++i)
					boneScale[i] = umath::lerp(1.0,boneScale[i],weight);
				boneScales->at(i) *= boneScale;
			}
			else
				boneScales->at(i) = uvec::lerp(boneScales->at(i),addScales->at(animBoneIdx),weight);
		}
	}
}
void BaseAnimatedComponent::BlendBonePoses(
	const std::vector<umath::Transform> &srcBonePoses,const std::vector<Vector3> *optSrcBoneScales,
	const std::vector<umath::Transform> &dstBonePoses,const std::vector<Vector3> *optDstBoneScales,
	std::vector<umath::Transform> &outBonePoses,std::vector<Vector3> *optOutBoneScales,
	Animation &anim,float interpFactor
) const
{
	auto numBones = umath::min(srcBonePoses.size(),dstBonePoses.size(),outBonePoses.size());
	auto numScales = (optSrcBoneScales && optDstBoneScales && optOutBoneScales) ? umath::min(optSrcBoneScales->size(),optDstBoneScales->size(),optOutBoneScales->size(),numBones) : 0;
	for(auto boneId=decltype(numBones){0u};boneId<numBones;++boneId)
	{
		auto &srcPose = srcBonePoses.at(boneId);
		auto dstPose = dstBonePoses.at(boneId);
		auto boneWeight = anim.GetBoneWeight(boneId);
		auto boneInterpFactor = boneWeight *interpFactor;
		auto &outPose = (outBonePoses.at(boneId) = srcPose);
		outPose.Interpolate(dstPose,boneInterpFactor);

		// Scaling
		if(boneId >= numScales)
			continue;
		optOutBoneScales->at(boneId) = uvec::lerp(optSrcBoneScales->at(boneId),optDstBoneScales->at(boneId) *boneWeight,interpFactor);
	}
}
void BaseAnimatedComponent::BlendBoneFrames(std::vector<umath::Transform> &tgt,std::vector<Vector3> *tgtScales,std::vector<umath::Transform> &add,std::vector<Vector3> *addScales,float blendScale) const
{
	if(blendScale == 0.f)
		return;
	for(unsigned int i=0;i<umath::min(tgt.size(),add.size());i++)
	{
		auto &pose = tgt.at(i);
		pose.Interpolate(add.at(i),blendScale);
		if(tgtScales != nullptr && addScales != nullptr)
			tgtScales->at(i) = uvec::lerp(tgtScales->at(i),addScales->at(i),blendScale);
	}
}

static void get_global_bone_transforms(std::vector<umath::ScaledTransform> &transforms,std::unordered_map<uint32_t,std::shared_ptr<Bone>> &childBones,const umath::ScaledTransform &tParent={})
{
	for(auto &pair : childBones)
	{
		auto boneId = pair.first;
		auto &bone = pair.second;
		auto &t = transforms.at(boneId);
		t = tParent *t;
		get_global_bone_transforms(transforms,bone->children,t);
	}
}
void BaseAnimatedComponent::UpdateSkeleton()
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	m_processedBones = m_bones;
	get_global_bone_transforms(m_processedBones,skeleton.GetRootBones());
}
