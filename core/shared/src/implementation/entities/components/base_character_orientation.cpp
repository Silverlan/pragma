// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_character;

using namespace pragma;

void BaseCharacterComponent::SetCharacterOrientation(const Vector3 &up)
{
	if(m_orientationComponent)
		m_orientationComponent->SetUpDirection(up);

	auto evOnSetCharacterOrientation = CEOnSetCharacterOrientation {up};
	BroadcastEvent(baseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED, evOnSetCharacterOrientation);
}

void BaseCharacterComponent::UpdateOrientation()
{
	auto &ent = GetEntity();
	auto physComponent = ent.GetPhysicsComponent();
	auto physType = physComponent ? physComponent->GetPhysicsType() : physics::PhysicsType::None;
	if(physType != physics::PhysicsType::BoxController && physType != physics::PhysicsType::CapsuleController)
		return;
	auto normal = uvec::PRM_UP;
	if(m_orientationComponent)
		normal = m_orientationComponent->GetUpDirection();
	auto gravityComponent = ent.GetComponent<GravityComponent>();
	if(gravityComponent.valid())
		gravityComponent->SetGravityOverride(-normal);

	auto pTrComponent = ent.GetTransformComponent();
	auto forward = pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::PRM_FORWARD;
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
