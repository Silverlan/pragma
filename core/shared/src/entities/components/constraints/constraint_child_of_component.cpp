/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_child_of_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/logging.hpp"

using namespace pragma;
#pragma optimize("", off)
void ConstraintChildOfComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintChildOfComponent;
	using TAxisEnabled = bool;
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetLocationAxisEnabled(pragma::Axis::X, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsLocationAxisEnabled(pragma::Axis::X); }>(
		    "locationX", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetLocationAxisEnabled(pragma::Axis::Y, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsLocationAxisEnabled(pragma::Axis::Y); }>(
		    "locationY", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetLocationAxisEnabled(pragma::Axis::Z, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsLocationAxisEnabled(pragma::Axis::Z); }>(
		    "locationZ", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetRotationAxisEnabled(pragma::Axis::X, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsRotationAxisEnabled(pragma::Axis::X); }>(
		    "rotationX", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetRotationAxisEnabled(pragma::Axis::Y, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsRotationAxisEnabled(pragma::Axis::Y); }>(
		    "rotationY", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetRotationAxisEnabled(pragma::Axis::Z, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsRotationAxisEnabled(pragma::Axis::Z); }>(
		    "rotationZ", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetScaleAxisEnabled(pragma::Axis::X, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsScaleAxisEnabled(pragma::Axis::X); }>(
		    "scaleX", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetScaleAxisEnabled(pragma::Axis::Y, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsScaleAxisEnabled(pragma::Axis::Y); }>(
		    "scaleY", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo
		  = create_component_member_info<T, TAxisEnabled, [](const ComponentMemberInfo &, T &c, TAxisEnabled enabled) { c.SetScaleAxisEnabled(pragma::Axis::Z, enabled); }, [](const ComponentMemberInfo &, T &c, TAxisEnabled &value) { value = c.IsScaleAxisEnabled(pragma::Axis::Z); }>(
		    "scaleZ", true);
		registerMember(std::move(memberInfo));
	}
}

ConstraintChildOfComponent::ConstraintChildOfComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_locationEnabled {true, true, true}, m_rotationEnabled {true, true, true}, m_scaleEnabled {true, true, true} {}

void ConstraintChildOfComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(ConstraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintChildOfComponent::SetLocationAxisEnabled(pragma::Axis axis, bool enabled) { m_locationEnabled[umath::to_integral(axis)] = enabled; }
bool ConstraintChildOfComponent::IsLocationAxisEnabled(pragma::Axis axis) const { return m_locationEnabled[umath::to_integral(axis)]; }

void ConstraintChildOfComponent::SetRotationAxisEnabled(pragma::Axis axis, bool enabled) { m_rotationEnabled[umath::to_integral(axis)] = enabled; }
bool ConstraintChildOfComponent::IsRotationAxisEnabled(pragma::Axis axis) const { return m_rotationEnabled[umath::to_integral(axis)]; }

void ConstraintChildOfComponent::SetScaleAxisEnabled(pragma::Axis axis, bool enabled) { m_scaleEnabled[umath::to_integral(axis)] = enabled; }
bool ConstraintChildOfComponent::IsScaleAxisEnabled(pragma::Axis axis) const { return m_scaleEnabled[umath::to_integral(axis)]; }

void ConstraintChildOfComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintChildOfComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
}

void ConstraintChildOfComponent::UpdateAxisState()
{
	m_allAxesEnabled = true;
	constexpr auto num = umath::to_integral(pragma::Axis::Count);
	for(auto i = decltype(num) {0u}; i < num; ++i) {
		if(!m_locationEnabled[i] || !m_rotationEnabled[i] || !m_scaleEnabled[i]) {
			m_allAxesEnabled = false;
			break;
		}
	}
}

std::optional<pragma::EntityUComponentMemberRef> ConstraintChildOfComponent::FindPosePropertyReference(pragma::BaseEntityComponent &c, pragma::ComponentMemberIndex basePropIdx)
{
	auto *memberInfo = c.GetMemberInfo(basePropIdx);
	if(!memberInfo)
		return {};
	if(memberInfo->type == pragma::ents::EntityMemberType::Transform || memberInfo->type == pragma::ents::EntityMemberType::ScaledTransform)
		return pragma::EntityUComponentMemberRef {c.GetEntity(), c.GetComponentId(), memberInfo->GetName()};
	auto *metaData = memberInfo->FindTypeMetaData<pragma::ents::PoseComponentTypeMetaData>();
	if(!metaData)
		return {};
	auto *poseMemberInfo = c.FindMemberInfo(metaData->poseProperty);
	if(!poseMemberInfo)
		return {};
	return pragma::EntityUComponentMemberRef {c.GetEntity(), c.GetComponentId(), metaData->poseProperty};
}

void ConstraintChildOfComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;

	auto drivenObjPoseProp = FindPosePropertyReference(*constraintInfo->drivenObjectC, constraintInfo->drivenObjectPropIdx);
	auto driverPoseProp = FindPosePropertyReference(*constraintInfo->driverC, constraintInfo->driverPropIdx);

	if(!drivenObjPoseProp.has_value() || !driverPoseProp.has_value())
		return;
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	drivenObjPoseProp->UpdateMemberIndex(game);
	driverPoseProp->UpdateMemberIndex(game);

	umath::ScaledTransform parentPose;
	constraintInfo->driverC->GetTransformMemberPose(driverPoseProp->GetMemberIndex(), umath::CoordinateSpace::World, parentPose);

	umath::ScaledTransform curPose;
	constraintInfo->drivenObjectC->GetTransformMemberPose(drivenObjPoseProp->GetMemberIndex(), umath::CoordinateSpace::World, curPose);

	umath::ScaledTransform newPose = parentPose * curPose;

	auto &curPos = curPose.GetOrigin();
	auto &newPos = newPose.GetOrigin();
	auto &curScale = curPose.GetScale();
	auto &newScale = newPose.GetScale();
	if(!m_allAxesEnabled) {
		auto curAng = EulerAngles {curPose.GetRotation()};
		auto newAng = EulerAngles {newPose.GetRotation()};
		constexpr auto num = umath::to_integral(pragma::Axis::Count);
		for(auto i = decltype(num) {0u}; i < num; ++i) {
			if(!m_locationEnabled[i])
				newPos[i] = curPos[i];
			if(!m_rotationEnabled[i])
				newAng[i] = curAng[i];
			if(!m_scaleEnabled[i])
				newScale[i] = curScale[i];
		}
		newPose.SetRotation(uquat::create(newAng));
	}

	curPose.Interpolate(newPose, influence);
	constraintInfo->drivenObjectC->SetTransformMemberPose(drivenObjPoseProp->GetMemberIndex(), static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), curPose, true);
}
#pragma optimize("", on)
