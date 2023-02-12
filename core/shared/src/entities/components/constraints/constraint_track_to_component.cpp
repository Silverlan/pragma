/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_track_to_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/logging.hpp"

using namespace pragma;
#pragma optimize("", off)
void ConstraintTrackToComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintTrackToComponent;
	{
		auto memberInfo = create_component_member_info<T, TrackAxis, &T::SetTrackAxis, &T::GetTrackAxis>("trackAxis", TrackAxis::X);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, pragma::Axis, &T::SetUpAxis, &T::GetUpAxis>("upAxis", pragma::Axis::Y);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, bool, &T::SetTargetY, &T::ShouldTargetY>("targetY", false);
		registerMember(std::move(memberInfo));
	}
}

ConstraintTrackToComponent::ConstraintTrackToComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintTrackToComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(ConstraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
	BindEventUnhandled(ConstraintComponent::EVENT_ON_DRIVEN_OBJECT_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ResetDrivenRotation(); });
}
void ConstraintTrackToComponent::ResetDrivenRotation() { m_drivenObjectRotationInitialized = false; }
void ConstraintTrackToComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintTrackToComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
}

void ConstraintTrackToComponent::SetTrackAxis(TrackAxis axis) { m_trackAxis = axis; }
ConstraintTrackToComponent::TrackAxis ConstraintTrackToComponent::GetTrackAxis() const { return m_trackAxis; }

void ConstraintTrackToComponent::SetUpAxis(pragma::Axis upAxis) { m_upAxis = upAxis; }
pragma::Axis ConstraintTrackToComponent::GetUpAxis() const { return m_upAxis; }

void ConstraintTrackToComponent::SetTargetY(bool targetY) { m_targetY = targetY; }
bool ConstraintTrackToComponent::ShouldTargetY() const { return m_targetY; }

void ConstraintTrackToComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo)
		return;
	if(!m_drivenObjectRotationInitialized) {
		m_drivenObjectRotationInitialized = true;
		auto &memberInfo = *constraintInfo->drivenObjectC->GetMemberInfo(constraintInfo->drivenObjectPropIdx);
		auto *metaData = memberInfo.FindTypeMetaData<pragma::ents::PoseComponentTypeMetaData>();
		if(metaData) {
			auto *poseMemberInfo = constraintInfo->drivenObjectC->FindMemberInfo(metaData->poseProperty);
			auto *poseMetaData = poseMemberInfo ? poseMemberInfo->FindTypeMetaData<pragma::ents::PoseTypeMetaData>() : nullptr;
			if(!poseMemberInfo) {
				spdlog::trace("Unable to initialize track_to constraint '{}' with driven object property {} of driven object '{}': Property has no associated rotation.", GetEntity().ToString(), constraintInfo->drivenObjectPropIdx, constraintInfo->drivenObjectC->GetEntity().ToString());
				return;
			}
			m_drivenObjectRotation = {constraintInfo->drivenObjectC->GetEntity(), constraintInfo->drivenObjectC->GetComponentId(), poseMetaData->rotProperty};
		}
	}
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	m_drivenObjectRotation.UpdateMemberIndex(game);
	auto *drivenObjectRotC = m_drivenObjectRotation.GetComponent(game);
	if(!drivenObjectRotC || drivenObjectRotC != constraintInfo->drivenObjectC)
		return;
	auto idxDrivenObjectRot = m_drivenObjectRotation.GetMemberIndex();
	if(idxDrivenObjectRot == pragma::INVALID_COMPONENT_MEMBER_INDEX)
		return;

	Vector3 posDriven;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberPos(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriven, true);
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

	Vector3 up;
	switch(m_upAxis) {
	case pragma::Axis::X:
		break;
	case pragma::Axis::Y:
		break;
	case pragma::Axis::Z:
		break;
	default:
		up = uvec::FORWARD;
		break;
	}

	auto rot = uquat::create_look_rotation(dir, up);
	constraintInfo->drivenObjectC->SetTransformMemberRot(idxDrivenObjectRot, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rot, true);
}
#pragma optimize("", on)
