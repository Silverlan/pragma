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
util::EventReply ConstraintChildOfComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == ConstraintComponent::EVENT_ON_PARTICIPANTS_FLAGGED_DIRTY)
		SetPropertyInfosDirty();
	return util::EventReply::Unhandled;
}
void ConstraintChildOfComponent::SetLocationAxisEnabled(pragma::Axis axis, bool enabled)
{
	m_locationEnabled[umath::to_integral(axis)] = enabled;
	UpdateAxisState();
}
bool ConstraintChildOfComponent::IsLocationAxisEnabled(pragma::Axis axis) const { return m_locationEnabled[umath::to_integral(axis)]; }

void ConstraintChildOfComponent::SetRotationAxisEnabled(pragma::Axis axis, bool enabled)
{
	m_rotationEnabled[umath::to_integral(axis)] = enabled;
	UpdateAxisState();
}
bool ConstraintChildOfComponent::IsRotationAxisEnabled(pragma::Axis axis) const { return m_rotationEnabled[umath::to_integral(axis)]; }

void ConstraintChildOfComponent::SetScaleAxisEnabled(pragma::Axis axis, bool enabled)
{
	m_scaleEnabled[umath::to_integral(axis)] = enabled;
	UpdateAxisState();
}
bool ConstraintChildOfComponent::IsScaleAxisEnabled(pragma::Axis axis) const { return m_scaleEnabled[umath::to_integral(axis)]; }

const pragma::ComponentHandle<ConstraintComponent> &ConstraintChildOfComponent::GetConstraint() const { return m_constraintC; }
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

std::optional<pragma::EntityUComponentMemberRef> ConstraintChildOfComponent::FindPosePropertyReference(const pragma::BaseEntityComponent &c, pragma::ComponentMemberIndex basePropIdx)
{
	auto *memberInfo = c.GetMemberInfo(basePropIdx);
	if(!memberInfo)
		return {};
	if(memberInfo->type == pragma::ents::EntityMemberType::Transform || memberInfo->type == pragma::ents::EntityMemberType::ScaledTransform)
		return pragma::EntityUComponentMemberRef {c.GetEntity(), c.GetComponentId(), memberInfo->GetName()};
	// Not a pose property, but we'll also allow positional and rotational properties
	if(memberInfo->type == pragma::ents::EntityMemberType::Vector3 || memberInfo->type == pragma::ents::EntityMemberType::Quaternion || memberInfo->type == pragma::ents::EntityMemberType::EulerAngles)
		return pragma::EntityUComponentMemberRef {c.GetEntity(), c.GetComponentId(), memberInfo->GetName()};
	return {};
}

void ConstraintChildOfComponent::SetPropertyInfosDirty()
{
	if(m_drivenObjectPropertyInfo)
		m_drivenObjectPropertyInfo = {};
	if(m_driverPropertyInfo)
		m_driverPropertyInfo = {};
}

bool ConstraintChildOfComponent::UpdatePropertyInfos()
{
	if(!m_constraintC.valid())
		return false;
	if(m_drivenObjectPropertyInfo && m_driverPropertyInfo)
		return true;
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo)
		return false;
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto applyPropInfo = [&game](const pragma::BaseEntityComponent &objC, pragma::ComponentMemberIndex propIdx, std::optional<PropertyInfo> &propInfo) {
		auto drivenObjPoseProp = FindPosePropertyReference(objC, propIdx);
		if(drivenObjPoseProp) {
			auto &info = propInfo;
			info = PropertyInfo {};
			info->propertyRef = std::move(*drivenObjPoseProp);
			info->propertyRef.UpdateMemberIndex(game);
			auto *memInfo = info->propertyRef.GetMemberInfo(game);
			if(memInfo) {
				if(memInfo->type == pragma::ents::EntityMemberType::Vector3)
					info->type = Type::Position;
				else if(memInfo->type == pragma::ents::EntityMemberType::Quaternion || memInfo->type == pragma::ents::EntityMemberType::EulerAngles)
					info->type = Type::Rotation;
				else
					info->type = Type::Pose;
			}
			else
				info = {};
		}
		else if(propInfo)
			propInfo = {};
	};
	applyPropInfo(*constraintInfo->drivenObjectC, constraintInfo->drivenObjectPropIdx, m_drivenObjectPropertyInfo);
	applyPropInfo(*constraintInfo->driverC, constraintInfo->driverPropIdx, m_driverPropertyInfo);
	return m_drivenObjectPropertyInfo && m_driverPropertyInfo;
}

std::optional<umath::ScaledTransform> ConstraintChildOfComponent::CalcInversePose(umath::ScaledTransform &pose) const
{
	pragma::ComponentMemberIndex drivenPropertyIndex;
	ConstraintComponent::ConstraintParticipants constraintInfo;
	return CalcConstraintPose(&pose, true, drivenPropertyIndex, constraintInfo);
}

std::optional<umath::ScaledTransform> ConstraintChildOfComponent::CalcConstraintPose(umath::ScaledTransform *optPose, bool inverse, pragma::ComponentMemberIndex &outDrivenPropertyIndex, ConstraintComponent::ConstraintParticipants &outConstraintParticipants) const
{
	if(!const_cast<ConstraintChildOfComponent *>(this)->UpdatePropertyInfos())
		return {};
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();

	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	m_drivenObjectPropertyInfo->propertyRef.UpdateMemberIndex(game);
	m_driverPropertyInfo->propertyRef.UpdateMemberIndex(game);

	umath::ScaledTransform parentPose {};
	auto getPropertyValue = [](const PropertyInfo &propInfo, const BaseEntityComponent &c, umath::ScaledTransform &outPose) {
		switch(propInfo.type) {
		case Type::Pose:
			c.GetTransformMemberPose(propInfo.propertyRef.GetMemberIndex(), umath::CoordinateSpace::World, outPose);
			break;
		case Type::Position:
			{
				Vector3 pos {};
				c.GetTransformMemberPos(propInfo.propertyRef.GetMemberIndex(), umath::CoordinateSpace::World, pos);
				outPose.SetOrigin(pos);
				break;
			}
		case Type::Rotation:
			{
				Quat rot {};
				c.GetTransformMemberRot(propInfo.propertyRef.GetMemberIndex(), umath::CoordinateSpace::World, rot);
				outPose.SetRotation(rot);
				break;
			}
		}
	};
	getPropertyValue(*m_driverPropertyInfo, *constraintInfo->driverC, parentPose);

	umath::ScaledTransform curPose;
	if(optPose)
		curPose = *optPose;
	else
		getPropertyValue(*m_drivenObjectPropertyInfo, *constraintInfo->drivenObjectC, curPose);

	if(inverse)
		parentPose = parentPose.GetInverse();
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

	auto influence = m_constraintC->GetInfluence();
	curPose.Interpolate(newPose, influence);
	outDrivenPropertyIndex = m_drivenObjectPropertyInfo->propertyRef.GetMemberIndex();
	outConstraintParticipants = *constraintInfo;
	return curPose;
}

void ConstraintChildOfComponent::ApplyConstraint()
{
	pragma::ComponentMemberIndex drivenPropertyIndex;
	ConstraintComponent::ConstraintParticipants constraintInfo;
	auto newPose = CalcConstraintPose(nullptr, false, drivenPropertyIndex, constraintInfo);
	if(!newPose.has_value())
		return;
	switch(m_drivenObjectPropertyInfo->type) {
	case Type::Pose:
		constraintInfo.drivenObjectC->SetTransformMemberPose(drivenPropertyIndex, umath::CoordinateSpace::World, *newPose);
		break;
	case Type::Position:
		constraintInfo.drivenObjectC->SetTransformMemberPos(drivenPropertyIndex, umath::CoordinateSpace::World, newPose->GetOrigin());
	case Type::Rotation:
		constraintInfo.drivenObjectC->SetTransformMemberRot(drivenPropertyIndex, umath::CoordinateSpace::World, newPose->GetRotation());
	}
}
