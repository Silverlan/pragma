#include "stdafx_shared.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

bool BaseAnimatedComponent::GetAttachment(unsigned int attID,Vector3 *pos,EulerAngles *angles) const
{
	assert(pos != nullptr && angles != nullptr); // TODO: Use references
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return false;
	Attachment *att = hMdl->GetAttachment(attID);
	if(att == NULL)
		return false;
	*pos = Vector3(0,0,0);
	auto rot = uquat::identity();
	GetBonePosition(att->bone,*pos,rot);
	*angles = EulerAngles(rot);
	Vector3 forward,right,up;
	angles->GetOrientation(&forward,&right,&up);
	auto pTrComponent = ent.GetTransformComponent();
	auto scale = pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	*pos += att->offset.x *scale *forward;
	*pos += att->offset.y *scale *up;
	*pos += att->offset.z *scale *right;
	*angles += att->angles;
	return true;
}
bool BaseAnimatedComponent::GetAttachment(const std::string &name,Vector3 *pos,EulerAngles *angles) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return false;
	int attID = hMdl->LookupAttachment(name);
	if(attID == -1)
		return false;
	return GetAttachment(attID,pos,angles);
}
bool BaseAnimatedComponent::GetAttachment(unsigned int attID,Vector3 *pos,Quat *rot) const
{
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return false;
	Attachment *att = hMdl->GetAttachment(attID);
	if(att == NULL)
		return false;
	Quat rotBone;
	Vector3 posBone;
	if(GetLocalBonePosition(att->bone,posBone,rotBone) == false)
		return false;
	if(pos != nullptr)
	{
		auto pTrComponent = ent.GetTransformComponent();
		auto offset = att->offset;
		uvec::rotate(&offset,rotBone);
		*pos = posBone +offset;
		*pos *= pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	}
	if(rot != nullptr)
		*rot = rotBone *uquat::create(att->angles);
	return true;
}
bool BaseAnimatedComponent::GetAttachment(const std::string &name,Vector3 *pos,Quat *rot) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return false;
	return GetAttachment(hMdl->LookupAttachment(name),pos,rot);
}
void BaseAnimatedComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	CEMaintainAnimationMovement evData {disp};
	InvokeEventCallbacks(EVENT_MAINTAIN_ANIMATION_MOVEMENT,evData);
}

void BaseAnimatedComponent::SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale)
{
	auto npos = pos;
	auto nrot = rot;
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->WorldToOrigin(&npos,&nrot);
	SetLocalBonePosition(boneId,npos,nrot,scale);
}
void BaseAnimatedComponent::SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot)
{
	auto npos = pos;
	auto nrot = rot;
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->WorldToOrigin(&npos,&nrot);
	SetLocalBonePosition(boneId,npos,nrot);
}
void BaseAnimatedComponent::SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos)
{
	auto npos = pos;
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->WorldToOrigin(&npos);
	SetLocalBonePosition(boneId,npos);
}
void BaseAnimatedComponent::SetGlobalBoneRotation(UInt32 boneId,const Quat &rot)
{
	auto nrot = rot;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
		pTrComponent->WorldToLocal(&nrot);
	SetLocalBoneRotation(boneId,nrot);
}
