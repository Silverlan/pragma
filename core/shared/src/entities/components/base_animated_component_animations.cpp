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
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

void BaseAnimatedComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	CEMaintainAnimationMovement evData {disp};
	InvokeEventCallbacks(EVENT_MAINTAIN_ANIMATION_MOVEMENT,evData);
}

void BaseAnimatedComponent::SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale)
{
	auto pose = GetEntity().GetPose();
	pose = pose.GetInverse();
	pose *= umath::ScaledTransform{pos,rot,scale};
	auto npos = pose.GetOrigin();
	auto nrot = pose.GetRotation();
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->WorldToOrigin(&npos,&nrot);
	SetLocalBonePosition(boneId,npos,nrot,scale);
}
void BaseAnimatedComponent::SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot)
{
	auto pose = GetEntity().GetPose();
	pose = pose.GetInverse();
	pose *= umath::Transform{pos,rot};
	auto npos = pose.GetOrigin();
	auto nrot = pose.GetRotation();
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->WorldToOrigin(&npos,&nrot);
	SetLocalBonePosition(boneId,npos,nrot);
}
void BaseAnimatedComponent::SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos)
{
	auto pose = GetEntity().GetPose();
	pose = pose.GetInverse();
	pose *= umath::Transform{pos,uquat::identity()};
	auto npos = pose.GetOrigin();
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->WorldToOrigin(&npos);
	SetLocalBonePosition(boneId,npos);
}
void BaseAnimatedComponent::SetGlobalBoneRotation(UInt32 boneId,const Quat &rot)
{
	auto pose = GetEntity().GetPose();
	pose = pose.GetInverse();
	pose *= umath::Transform{Vector3{},rot};
	auto nrot = pose.GetRotation();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent)
		pTrComponent->WorldToLocal(&nrot);
	SetLocalBoneRotation(boneId,nrot);
}
