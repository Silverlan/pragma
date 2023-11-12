/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_look_at_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

void ConstraintLookAtComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintLookAtComponent;
	{
		auto memberInfo = create_component_member_info<T, TrackAxis, &T::SetTrackAxis, &T::GetTrackAxis>("trackAxis", TrackAxis::Z);
		registerMember(std::move(memberInfo));
	}
}

ConstraintLookAtComponent::ConstraintLookAtComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintLookAtComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(ConstraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
	BindEventUnhandled(ConstraintComponent::EVENT_ON_DRIVEN_OBJECT_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ResetDrivenRotation(); });
}
void ConstraintLookAtComponent::ResetDrivenRotation() { m_drivenObjectRotationInitialized = false; }
void ConstraintLookAtComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintLookAtComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
}

void ConstraintLookAtComponent::SetTrackAxis(TrackAxis axis) { m_trackAxis = axis; }
ConstraintLookAtComponent::TrackAxis ConstraintLookAtComponent::GetTrackAxis() const { return m_trackAxis; }

static std::array<Quat, umath::to_integral(ConstraintLookAtComponent::TrackAxis::Count)> g_axisRotations {
  uquat::create(EulerAngles {0.f, -90.f, 0.f}), // x+
  uquat::create(EulerAngles {90.f, 0.f, 0.f}),  // y+
  uquat::create(EulerAngles {0.f, 0.f, 0.f}),   // z+
  uquat::create(EulerAngles {0.f, 90.f, 0.f}),  // x-
  uquat::create(EulerAngles {-90.f, 0.f, 0.f}), // y-
  uquat::create(EulerAngles {0.f, 180.f, 0.f})  // z-
};

void rotate_towards_axis(Quat &rotation, const Vector3 &targetAxis, pragma::ConstraintLookAtComponent::TrackAxis eAxis)
{
	auto rotDriven = uquat::create_look_rotation(targetAxis, uquat::up(rotation));
	rotDriven = rotDriven * g_axisRotations[umath::to_integral(eAxis)];
	rotation = rotDriven;
}

void ConstraintLookAtComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;
	if(!m_drivenObjectRotationInitialized) {
		m_drivenObjectRotationInitialized = true;
		auto &memberInfo = *constraintInfo->drivenObjectC->GetMemberInfo(constraintInfo->drivenObjectPropIdx);
		auto *metaData = memberInfo.FindTypeMetaData<pragma::ents::PoseTypeMetaData>();
		if(!metaData) {
			spdlog::trace("Unable to initialize look_at constraint '{}' with driven object property {} of driven object '{}': Property has no pose type meta data.", GetEntity().ToString(), constraintInfo->drivenObjectPropIdx, constraintInfo->drivenObjectC->GetEntity().ToString());
			return;
		}
		m_drivenObjectPosition = {constraintInfo->drivenObjectC->GetEntity(), constraintInfo->drivenObjectC->GetComponentId(), metaData->posProperty};
		m_drivenObjectRotation = {constraintInfo->drivenObjectC->GetEntity(), constraintInfo->drivenObjectC->GetComponentId(), metaData->rotProperty};
	}
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	m_drivenObjectPosition.UpdateMemberIndex(game);
	m_drivenObjectRotation.UpdateMemberIndex(game);
	auto *drivenObjectPosC = m_drivenObjectPosition.GetComponent(game);
	if(!drivenObjectPosC || drivenObjectPosC != constraintInfo->drivenObjectC.get())
		return;
	auto *drivenObjectRotC = m_drivenObjectRotation.GetComponent(game);
	if(!drivenObjectRotC || drivenObjectRotC != constraintInfo->drivenObjectC.get())
		return;
	auto idxDrivenObjectPos = m_drivenObjectPosition.GetMemberIndex();
	if(idxDrivenObjectPos == pragma::INVALID_COMPONENT_MEMBER_INDEX)
		return;
	auto idxDrivenObjectRot = m_drivenObjectRotation.GetMemberIndex();
	if(idxDrivenObjectRot == pragma::INVALID_COMPONENT_MEMBER_INDEX)
		return;

	Vector3 posDriven;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberPos(idxDrivenObjectPos, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriven);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}

	Vector3 posDriver;
	res = constraintInfo->driverC->GetTransformMemberPos(constraintInfo->driverPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDriverSpace()), posDriver);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driver of constraint '{}'.", constraintInfo->driverPropIdx, GetEntity().ToString());
		return;
	}

	auto dir = posDriver - posDriven;
	auto l = uvec::length(dir);
	if(l < 0.0001f)
		dir = uvec::FORWARD;
	else
		dir /= l;

	Quat curRot;
	res = constraintInfo->drivenObjectC->GetTransformMemberRot(idxDrivenObjectRot, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), curRot);

	auto rot = curRot;
	rotate_towards_axis(rot, dir, m_trackAxis);

	if(res)
		rot = uquat::slerp(curRot, rot, influence);

	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberRot(idxDrivenObjectRot, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rot);
}
