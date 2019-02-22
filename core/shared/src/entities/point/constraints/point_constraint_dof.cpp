#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/point/constraints/point_constraint_dof.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physconstraint.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/physcollisionobject.h"
#include <sharedutils/util.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BasePointConstraintDoFComponent::Initialize()
{
	BasePointConstraintComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"limit_lin_lower",false))
			m_kvLimLinLower = uvec::create(kvData.value);
		else if(ustring::compare(kvData.key,"limit_lin_upper",false))
			m_kvLimLinUpper = uvec::create(kvData.value);
		else if(ustring::compare(kvData.key,"limit_ang_lower",false))
			m_kvLimAngLower = uvec::create(kvData.value);
		else if(ustring::compare(kvData.key,"limit_ang_upper",false))
			m_kvLimAngUpper = uvec::create(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BasePointConstraintDoFComponent::InitializeConstraint(BaseEntity *src,BaseEntity *tgt)
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
	//auto &posThis = entThis.GetPosition();
	auto pTrComponent = entThis.GetTransformComponent();
	auto dir = pTrComponent.valid() ? pTrComponent->GetForward() : uvec::FORWARD;

	auto &bodies = physTgt->GetRigidBodies();
	for(auto it=bodies.begin();it!=bodies.end();++it)
	{
		auto &bodyTgt = *it;
		if(bodyTgt.IsValid())
		{
			auto posTgt = bodyTgt->GetPos();
			auto *dof = physEnv->CreateDoFConstraint(bodySrc,Vector3(0.f,0.f,0.f),uquat::identity(),static_cast<PhysRigidBody*>(bodyTgt.get()),Vector3(0.f,50.f,0.f),uquat::identity());
			if(dof != nullptr)
			{
				//dof->SetLinearLimit(Vector3(1.f,1.f,1.f),Vector3(-1.f,-1.f,-1.f));
				dof->SetAngularLimit(Vector3(M_PI,M_PI,M_PI),Vector3(-M_PI,-M_PI,-M_PI));
				//dof->SetLinearLimit(m_kvLimLinLower,m_kvLimLinUpper);
				//dof->SetAngularLimit(m_kvLimAngLower,m_kvLimAngUpper);
				m_constraints.push_back(dof->GetHandle());
			}
		}
	}
}