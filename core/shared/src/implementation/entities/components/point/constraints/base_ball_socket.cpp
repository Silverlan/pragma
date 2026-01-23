// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.point.constraints.base_ball_socket;

using namespace pragma;

void BasePointConstraintBallSocketComponent::Initialize() { BasePointConstraintComponent::Initialize(); }

void BasePointConstraintBallSocketComponent::OnEntitySpawn()
{
	BasePointConstraintComponent::OnEntitySpawn();
	BasePointConstraintComponent::InitializeConstraint();
}

void BasePointConstraintBallSocketComponent::InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt)
{
	auto pPhysComponentTgt = tgt->GetPhysicsComponent();
	auto *physTgt = pPhysComponentTgt ? dynamic_cast<physics::RigidPhysObj *>(pPhysComponentTgt->GetPhysicsObject()) : nullptr;
	if(physTgt == nullptr)
		return;
	auto pPhysComponentSrc = src->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc ? dynamic_cast<physics::RigidPhysObj *>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
	if(physSrc == nullptr)
		return;
	auto *bodySrc = physSrc->GetRigidBody();
	if(bodySrc == nullptr)
		return;
	auto &entThis = GetEntity();
	auto *state = entThis.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto pTrComponent = entThis.GetTransformComponent();
	auto posThis = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	auto dir = pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::PRM_FORWARD;

	auto &bodies = physTgt->GetRigidBodies();
	for(auto it = bodies.begin(); it != bodies.end(); ++it) {
		auto &bodyTgt = *it;
		if(bodyTgt.IsValid()) {
			auto posTgt = bodyTgt->GetPos();
			auto ballSocket = physEnv->CreateBallSocketConstraint(*bodyTgt, posThis - posTgt, *bodySrc, posThis);
			if(ballSocket.IsValid())
				ballSocket->SetEntity(GetEntity());
			//if(ballSocket != nullptr)
			//	m_constraints.push_back(ballSocket->GetHandle());
		}
	}
}
