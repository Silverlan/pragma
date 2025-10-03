// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "sharedutils/util_shared_handle.hpp"

#include "mathutil/uvec.h"

module pragma.shared;

import :entities.components.point.constraints.base_fixed;

using namespace pragma;

void BasePointConstraintFixedComponent::InitializeConstraint(BaseEntity *src, BaseEntity *tgt)
{
	auto pPhysComponentTgt = tgt->GetPhysicsComponent();
	auto *physTgt = pPhysComponentTgt ? dynamic_cast<RigidPhysObj *>(pPhysComponentTgt->GetPhysicsObject()) : nullptr;
	if(physTgt == nullptr)
		return;
	auto pPhysComponentSrc = src->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc ? dynamic_cast<RigidPhysObj *>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
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
	auto dir = pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::FORWARD;

	auto &bodies = physTgt->GetRigidBodies();
	m_constraints.reserve(bodies.size());
	for(auto it = bodies.begin(); it != bodies.end(); ++it) {
		auto &bodyTgt = *it;
		if(bodyTgt.IsValid()) {
			auto posSrc = posThis - bodySrc->GetPos();
			auto posTgt = posThis - bodyTgt->GetPos();
			auto fixed = physEnv->CreateFixedConstraint(*bodyTgt, posTgt, uquat::identity(), *bodySrc, posSrc, uquat::identity());
			if(fixed != nullptr) {
				fixed->SetEntity(GetEntity());
				m_constraints.push_back(util::shared_handle_cast<pragma::physics::IFixedConstraint, pragma::physics::IConstraint>(fixed));
			}
		}
	}
}
