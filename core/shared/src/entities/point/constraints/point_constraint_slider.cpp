#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/point/constraints/point_constraint_slider.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/constraint.hpp"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/collision_object.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include <sharedutils/util.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/game/game_coordinate_system.hpp"
#include <glm/gtx/euler_angles.hpp>

using namespace pragma;

void BasePointConstraintSliderComponent::Initialize()
{
	BasePointConstraintComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"limit_low",false))
			m_kvLimitLinLow = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"limit_high",false))
			m_kvLimitLinHigh = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BasePointConstraintSliderComponent::InitializeConstraint(BaseEntity *src,BaseEntity *tgt)
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

	auto dir = m_posTarget -posThis;
	auto l = uvec::length(dir);
	if(l > 0.f)
		dir /= l;

	auto forward = uvec::get_rotation(Vector3(0.f,0.f,1.f),dir);
	uquat::normalize(forward);
	auto up = Vector3(0.f,1.f,0.f) *forward;
	auto right = Vector3(-1.f,0.f,0.f) *forward;
	auto rot = uquat::create(dir,right,up);

	auto &bodies = physTgt->GetRigidBodies();
	for(auto &hBody : bodies)
	{
		if(hBody.IsValid() == false)
			continue;
		auto *rigidBody0 = hBody.Get();
		auto *rigidBody1 = bodySrc;
		auto pivot0 = Vector3{};
		auto pivot1 = rigidBody0->GetPos();
		auto rotation0 = -rot *uquat::create(EulerAngles(0,90,0));
		auto rotation1 = rotation0;
		rotation0 = tgt->GetTransformComponent()->GetOrientation() *rotation0;

		auto slider = physEnv->CreateSliderConstraint(*rigidBody0,pivot0,rotation0,*rigidBody1,pivot1,rotation1);
		if(slider != nullptr)
		{
			slider->SetEntity(GetEntity());
			m_constraints.push_back(util::shared_handle_cast<pragma::physics::ISliderConstraint,pragma::physics::IConstraint>(slider));
		}
	}
}

