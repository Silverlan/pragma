#include "stdafx_shared.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

static void get_local_bone_position(std::vector<Transform> &transforms,std::shared_ptr<Bone> &bone,const Vector3 &fscale={1.f,1.f,1.f},Vector3 *pos=nullptr,Quat *rot=nullptr,Vector3 *scale=nullptr)
{
	std::function<void(std::shared_ptr<Bone>&,Vector3*,Quat*,Vector3*)> apply;
	apply = [&transforms,&apply,fscale](std::shared_ptr<Bone> &bone,Vector3 *pos,Quat *rot,Vector3 *scale) {
		auto parent = bone->parent.lock();
		if(parent != nullptr)
			apply(parent,pos,rot,scale);
		auto &tParent = transforms[bone->ID];
		auto &posParent = tParent.GetPosition();
		auto &rotParent = tParent.GetOrientation();
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
static void get_local_bone_position(const std::shared_ptr<Model> &mdl,std::vector<Transform> &transforms,std::shared_ptr<Bone> &bone,const Vector3 &fscale={1.f,1.f,1.f},Vector3 *pos=nullptr,Quat *rot=nullptr,Vector3 *scale=nullptr)
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
const std::vector<Transform> &BaseAnimatedComponent::GetBoneTransforms() const {return const_cast<BaseAnimatedComponent&>(*this).GetBoneTransforms();}
std::vector<Transform> &BaseAnimatedComponent::GetBoneTransforms() {return m_bones;}
const std::vector<Transform> &BaseAnimatedComponent::GetProcessedBoneTransforms() const {return const_cast<BaseAnimatedComponent&>(*this).GetProcessedBoneTransforms();}
std::vector<Transform> &BaseAnimatedComponent::GetProcessedBoneTransforms() {return m_processedBones;}

Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 &scale) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetPosition();
	rot = m_bones[boneId].GetOrientation();
	scale = m_bones[boneId].GetScale();
	return true;
}
Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetPosition();
	rot = m_bones[boneId].GetOrientation();
	return true;
}
Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetPosition();
	return true;
}
Bool BaseAnimatedComponent::GetBoneRotation(UInt32 boneId,Quat &rot) const
{
	if(boneId >= m_bones.size())
		return false;
	rot = m_bones[boneId].GetOrientation();
	return true;
}
Bool BaseAnimatedComponent::GetBonePosition(UInt32 boneId,Vector3 &pos,EulerAngles &ang) const
{
	if(boneId >= m_bones.size())
		return false;
	pos = m_bones[boneId].GetPosition();
	ang = EulerAngles(m_bones[boneId].GetOrientation());
	return true;
}
Bool BaseAnimatedComponent::GetBoneAngles(UInt32 boneId,EulerAngles &ang) const
{
	if(boneId >= m_bones.size())
		return false;
	ang = EulerAngles(m_bones[boneId].GetOrientation());
	return true;
}
const Vector3 *BaseAnimatedComponent::GetBonePosition(UInt32 boneId) const
{
	if(boneId >= m_bones.size())
		return nullptr;
	return &m_bones[boneId].GetPosition();
}
const Quat *BaseAnimatedComponent::GetBoneRotation(UInt32 boneId) const
{
	if(boneId >= m_bones.size())
		return nullptr;
	return &m_bones[boneId].GetOrientation();
}
// See also lanimation.cpp
Bool BaseAnimatedComponent::GetLocalBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 *scale) const
{
	if(boneId >= m_bones.size())
		return false;
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return false;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return false;
	if(scale == nullptr)
	{
		// Faster, but doesn't work with scales
		auto &t = m_bones[boneId];
		pos = t.GetPosition();
		rot = t.GetOrientation();
		auto parent = bone->parent.lock();
		while(parent != nullptr)
		{
			auto &tParent = m_bones[parent->ID];
			auto &posParent = tParent.GetPosition();
			auto &rotParent = tParent.GetOrientation();
			uvec::rotate(&pos,rotParent);
			pos += posParent;
			rot = rotParent *rot;
			parent = parent->parent.lock();
		}
		return true;
	}

	auto &t = m_bones[boneId];
	std::function<void(const std::shared_ptr<Bone>&,Vector3&,Quat&,Vector3*)> fIterateHierarchy = nullptr;
	fIterateHierarchy = [this,&fIterateHierarchy](const std::shared_ptr<Bone> &bone,Vector3 &pos,Quat &rot,Vector3 *scale) {
		auto &t = m_bones.at(bone->ID);
		if(bone->parent.expired() == true)
		{
			pos = t.GetPosition();
			rot = t.GetOrientation();
			if(scale != nullptr)
				*scale = t.GetScale();
			return;
		}
		fIterateHierarchy(bone->parent.lock(),pos,rot,scale);
		auto bonePos = t.GetPosition();
		auto &boneRot = t.GetOrientation();
		uvec::rotate(&bonePos,rot);
		if(scale != nullptr)
			bonePos *= *scale;
		pos += bonePos;

		if(scale != nullptr)
			*scale *= t.GetScale();

		rot = rot *boneRot;
	};
	Vector3 nscale(1.f,1.f,1.f);
	fIterateHierarchy(bone,pos,rot,&nscale);
	if(scale != nullptr)
		*scale = nscale;
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
	if(pTrComponent.expired())
		return true;
	uvec::local_to_world(pTrComponent->GetOrigin(),pTrComponent->GetOrientation(),pos,rot);//uvec::local_to_world(GetOrigin(),GetOrientation(),pos,rot);
	return true;
}
Bool BaseAnimatedComponent::GetGlobalBonePosition(UInt32 boneId,Vector3 &pos) const
{
	if(GetLocalBonePosition(boneId,pos) == false)
		return false;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.expired())
		return true;
	uvec::local_to_world(pTrComponent->GetOrigin(),pTrComponent->GetOrientation(),pos);//uvec::local_to_world(GetOrigin(),GetOrientation(),pos);
	return true;
}
Bool BaseAnimatedComponent::GetGlobalBoneRotation(UInt32 boneId,Quat &rot) const
{
	if(GetLocalBoneRotation(boneId,rot) == false)
		return false;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.expired())
		return true;
	uvec::local_to_world(pTrComponent->GetOrientation(),rot);
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
	m_bones[boneId].SetPosition(pos);
	m_bones[boneId].SetOrientation(rot);
	if(scale != nullptr)
		m_bones[boneId].SetScale(*scale);
	m_bones[boneId].UpdateMatrix();
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
	m_bones[boneId].SetPosition(pos);
	m_bones[boneId].UpdateMatrix();

	CEOnBoneTransformChanged evData {boneId,&pos,nullptr,nullptr};
	InvokeEventCallbacks(EVENT_ON_BONE_TRANSFORM_CHANGED,evData);
}
void BaseAnimatedComponent::SetBoneRotation(UInt32 boneId,const Quat &rot)
{
	if(boneId >= m_bones.size())
		return;
	m_bones[boneId].SetOrientation(rot);
	m_bones[boneId].UpdateMatrix();

	CEOnBoneTransformChanged evData {boneId,nullptr,&rot,nullptr};
	InvokeEventCallbacks(EVENT_ON_BONE_TRANSFORM_CHANGED,evData);
}

void BaseAnimatedComponent::SetLocalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale)
{
	if(boneId >= m_bones.size())
		return;
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
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
	get_local_bone_position(hModel,m_bones,bone,pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},&npos,&nrot,&nscale);
	SetBonePosition(boneId,npos,nrot,nscale);
}
void BaseAnimatedComponent::SetLocalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot)
{
	if(boneId >= m_bones.size())
		return;
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	auto npos = pos;
	auto nrot = rot;
	auto pTrComponent = GetEntity().GetTransformComponent();
	get_local_bone_position(hModel,m_bones,bone,pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},&npos,&nrot);
	SetBonePosition(boneId,npos,nrot);
}
void BaseAnimatedComponent::SetLocalBonePosition(UInt32 boneId,const Vector3 &pos)
{
	if(boneId >= m_bones.size())
		return;
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	auto npos = pos;
	auto pTrComponent = GetEntity().GetTransformComponent();
	get_local_bone_position(hModel,m_bones,bone,pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},&npos);
	SetBonePosition(boneId,npos);
}
void BaseAnimatedComponent::SetLocalBoneRotation(UInt32 boneId,const Quat &rot)
{
	if(boneId >= m_bones.size())
		return;
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	auto nrot = rot;
	auto pTrComponent = GetEntity().GetTransformComponent();
	get_local_bone_position(hModel,m_bones,bone,pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f},nullptr,&nrot);
	SetBoneRotation(boneId,nrot);
}

const Mat4 *BaseAnimatedComponent::GetBoneMatrix(unsigned int boneID) const
{
	if(boneID >= m_bones.size())
		return nullptr;
	return &m_bones[boneID].GetTransformationMatrix();
}
bool BaseAnimatedComponent::ShouldUpdateBones() const
{
	if(IsPlayingAnimation())
		return true;
	CEShouldUpdateBones evData {};
	return InvokeEventCallbacks(EVENT_SHOULD_UPDATE_BONES,evData) == util::EventReply::Handled && evData.shouldUpdate;
}

void BaseAnimatedComponent::TransformBoneFrames(std::vector<Orientation> &boneOrientations,std::vector<Vector3> *boneScales,Animation &anim,Frame *frameBlend,bool bAdd)
{
	for(unsigned int i=0;i<boneOrientations.size();i++)
	{
		Orientation &orientation = boneOrientations[i];
		Vector3 *posBlend = frameBlend->GetBonePosition(i);
		Quat *rotBlend = frameBlend->GetBoneOrientation(i);
		auto weight = anim.GetBoneWeight(i);
		if(posBlend != NULL && rotBlend != NULL)
		{
			if(bAdd == true)
			{
				orientation.pos += *posBlend *weight;
				orientation.rot = orientation.rot *glm::slerp(uquat::identity(),*rotBlend,weight);//orientation.rot *(*rotBlend);
			}
			else
			{
				orientation.pos = orientation.pos +(*posBlend -orientation.pos) *weight;
				orientation.rot = glm::slerp(orientation.rot,*rotBlend,weight);
			}
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
void BaseAnimatedComponent::TransformBoneFrames(std::vector<Orientation> &tgt,std::vector<Vector3> *boneScales,const std::shared_ptr<Animation> &anim,std::vector<Orientation> &add,std::vector<Vector3> *addScales,bool bAdd)
{
	for(auto i=decltype(tgt.size()){0};i<tgt.size();++i)
	{
		auto animBoneIdx = anim->LookupBone(i);
		if(animBoneIdx == -1 || animBoneIdx >= add.size())
			continue;
		auto &orientation = tgt.at(i);
		auto weight = anim->GetBoneWeight(i);
		if(bAdd == true)
		{
			orientation.pos += add.at(animBoneIdx).pos *weight;
			orientation.rot = orientation.rot *glm::slerp(uquat::identity(),add.at(animBoneIdx).rot,weight);
		}
		else
		{
			orientation.pos = orientation.pos +(add.at(animBoneIdx).pos -orientation.pos) *weight;
			orientation.rot = glm::slerp(orientation.rot,add.at(animBoneIdx).rot,weight);
			//orientation.pos = add.at(animBoneIdx).pos *weight;
			//orientation.rot = glm::slerp(uquat::identity(),add.at(animBoneIdx).rot,weight);
		}
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
void BaseAnimatedComponent::BlendBoneFrames(std::vector<Orientation> &boneOrientations,std::vector<Vector3> *boneScales,Animation &anim,Frame *frameBlend,float blendScale) const
{
	for(unsigned int i=0;i<boneOrientations.size();i++)
	{
		Orientation &orientation = boneOrientations[i];
		Vector3 *posBlend = frameBlend->GetBonePosition(i);
		auto *rotBlend = frameBlend->GetBoneOrientation(i);
		auto boneBlendScale = anim.GetBoneWeight(i);
		if(posBlend != NULL && rotBlend != NULL)
		{
			boneBlendScale *= blendScale;
			orientation.pos = orientation.pos +(*posBlend -orientation.pos) *boneBlendScale;
			orientation.rot = glm::slerp(orientation.rot,*rotBlend,boneBlendScale);
		}
		if(boneScales != nullptr)
		{
			auto *scaleBlend = frameBlend->GetBoneScale(i);
			if(scaleBlend != nullptr)
				boneScales->at(i) = uvec::lerp(boneScales->at(i),*scaleBlend *boneBlendScale,blendScale);
		}
	}
}
void BaseAnimatedComponent::BlendBoneFrames(std::vector<Orientation> &tgt,std::vector<Vector3> *tgtScales,std::vector<Orientation> &add,std::vector<Vector3> *addScales,float blendScale) const
{
	if(blendScale == 0.f)
		return;
	for(unsigned int i=0;i<umath::min(tgt.size(),add.size());i++)
	{
		Orientation &orientation = tgt[i];
		orientation.pos = orientation.pos +(add[i].pos -orientation.pos) *blendScale;
		orientation.rot = glm::slerp(orientation.rot,add[i].rot,blendScale);
		if(tgtScales != nullptr && addScales != nullptr)
			tgtScales->at(i) = uvec::lerp(tgtScales->at(i),addScales->at(i),blendScale);
	}
}

static void get_global_bone_transforms(std::vector<Transform> &transforms,std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,const Vector3 &origin=Vector3(0.f,0.f,0.f),const Quat &originRot=uquat::identity(),const Vector3 &originScale=Vector3(1.f,1.f,1.f))
{
	for(auto it=bones.begin();it!=bones.end();++it)
	{
		auto &bone = it->second;
		auto boneId = it->first;
		auto &t = transforms[boneId];
		
		auto pos = t.GetPosition();
		uvec::rotate(&pos,originRot);
		pos *= originScale;
		pos += origin;
		t.SetPosition(pos);

		t.SetScale(t.GetScale() *originScale);

		auto rot = t.GetOrientation();
		rot = originRot *rot;
		t.SetOrientation(rot);
		t.UpdateMatrix();

		get_global_bone_transforms(transforms,bone->children,pos,rot,t.GetScale());
	}
}
void BaseAnimatedComponent::UpdateSkeleton()
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return;
	auto &skeleton = hModel->GetSkeleton();
	m_processedBones = m_bones;
	get_global_bone_transforms(m_processedBones,skeleton.GetRootBones());
}
