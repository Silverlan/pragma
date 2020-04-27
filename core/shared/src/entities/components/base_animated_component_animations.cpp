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
