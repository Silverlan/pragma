/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/point/constraints/point_constraint_ballsocket.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/constraint.hpp"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/collision_object.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"

using namespace pragma;

void BasePointConstraintBallSocketComponent::Initialize()
{
	BasePointConstraintComponent::Initialize();
}

void BasePointConstraintBallSocketComponent::OnEntitySpawn()
{
	BasePointConstraintComponent::OnEntitySpawn();
	BasePointConstraintComponent::InitializeConstraint();
}

void BasePointConstraintBallSocketComponent::InitializeConstraint(BaseEntity *src,BaseEntity *tgt)
{
	auto pPhysComponentTgt = tgt->GetPhysicsComponent();
	auto *physTgt = pPhysComponentTgt ? dynamic_cast<RigidPhysObj*>(pPhysComponentTgt->GetPhysicsObject()) : nullptr;
	if(physTgt == nullptr)
		return;
	auto pPhysComponentSrc = src->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc ? dynamic_cast<RigidPhysObj*>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
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
	auto posThis = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
	auto dir = pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::FORWARD;

	auto &bodies = physTgt->GetRigidBodies();
	for(auto it=bodies.begin();it!=bodies.end();++it)
	{
		auto &bodyTgt = *it;
		if(bodyTgt.IsValid())
		{
			auto posTgt = bodyTgt->GetPos();
			auto ballSocket = physEnv->CreateBallSocketConstraint(*bodyTgt,posThis -posTgt,*bodySrc,posThis);
			if(ballSocket.IsValid())
				ballSocket->SetEntity(GetEntity());
			//if(ballSocket != nullptr)
			//	m_constraints.push_back(ballSocket->GetHandle());
		}
	}
}
