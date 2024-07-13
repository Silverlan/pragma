/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/orientation_component.hpp"

using namespace pragma;

void BaseCharacterComponent::SetCharacterOrientation(const Vector3 &up)
{
	if(m_orientationComponent)
		m_orientationComponent->SetUpDirection(up);

	auto evOnSetCharacterOrientation = CEOnSetCharacterOrientation {up};
	BroadcastEvent(EVENT_ON_CHARACTER_ORIENTATION_CHANGED, evOnSetCharacterOrientation);
}

void BaseCharacterComponent::UpdateOrientation()
{
	auto &ent = GetEntity();
	auto physComponent = ent.GetPhysicsComponent();
	auto physType = physComponent ? physComponent->GetPhysicsType() : PHYSICSTYPE::NONE;
	if(physType != PHYSICSTYPE::BOXCONTROLLER && physType != PHYSICSTYPE::CAPSULECONTROLLER)
		return;
	auto normal = uvec::UP;
	if(m_orientationComponent)
		normal = m_orientationComponent->GetUpDirection();
	auto gravityComponent = ent.GetComponent<pragma::GravityComponent>();
	if(gravityComponent.valid())
		gravityComponent->SetGravityOverride(-normal);

	auto pTrComponent = ent.GetTransformComponent();
	auto forward = pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::FORWARD;
	forward = forward - uvec::project(forward, normal);
	uvec::normalize(&forward);

	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto pTimeScaleComponent = ent.GetTimeScaleComponent();
	auto t = game->DeltaTickTime() * (pTimeScaleComponent.valid() ? pTimeScaleComponent->GetEffectiveTimeScale() : 1.f);
	auto curRot = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	auto dstRot = uquat::create_look_rotation(forward, normal);
	const auto speed = 4.f; // TODO
	dstRot = uquat::slerp(curRot, dstRot, t * speed);
	if(pTrComponent != nullptr)
		pTrComponent->SetRotation(dstRot);
	auto *phys = physComponent ? physComponent->GetPhysicsObject() : nullptr;
	if(phys != nullptr && phys->IsController()) {
		auto *col = phys->GetCollisionObject();
		if(col != nullptr)
			col->SetRotation(dstRot);
	}
}
