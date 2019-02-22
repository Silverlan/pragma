#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/point/constraints/point_constraint_ballsocket.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physconstraint.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/physcollisionobject.h"
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
	auto *physTgt = pPhysComponentTgt.valid() ? dynamic_cast<RigidPhysObj*>(pPhysComponentTgt->GetPhysicsObject()) : nullptr;
	if(physTgt == nullptr)
		return;
	auto pPhysComponentSrc = src->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc.valid() ? dynamic_cast<RigidPhysObj*>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
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
	auto posThis = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto dir = pTrComponent.valid() ? pTrComponent->GetForward() : uvec::FORWARD;

	auto &bodies = physTgt->GetRigidBodies();
	for(auto it=bodies.begin();it!=bodies.end();++it)
	{
		auto &bodyTgt = *it;
		if(bodyTgt.IsValid())
		{
			auto posTgt = bodyTgt->GetPos();
			auto *ballSocket = physEnv->CreateBallSocketConstraint(static_cast<PhysRigidBody*>(bodyTgt.get()),posThis -posTgt,bodySrc,posThis);
			//if(ballSocket != nullptr)
			//	m_constraints.push_back(ballSocket->GetHandle());
		}
	}
}
