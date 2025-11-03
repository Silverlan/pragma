// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/core.hpp"

#include "pragma/logging.hpp"

module pragma.shared;

import :entities.components.constraints.limit_location;

using namespace pragma;
static void set_x_axis_minimum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float limit) { component.SetMinimum(pragma::Axis::X, limit); }
static void set_y_axis_minimum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float limit) { component.SetMinimum(pragma::Axis::Y, limit); }
static void set_z_axis_minimum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float limit) { component.SetMinimum(pragma::Axis::Z, limit); }
static void get_x_axis_minimum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float &outValue) { outValue = component.GetMinimum(pragma::Axis::X); }
static void get_y_axis_minimum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float &outValue) { outValue = component.GetMinimum(pragma::Axis::Y); }
static void get_z_axis_minimum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float &outValue) { outValue = component.GetMinimum(pragma::Axis::Z); }

static void set_x_axis_minimum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool enabled) { component.SetMinimumEnabled(pragma::Axis::X, enabled); }
static void set_y_axis_minimum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool enabled) { component.SetMinimumEnabled(pragma::Axis::Y, enabled); }
static void set_z_axis_minimum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool enabled) { component.SetMinimumEnabled(pragma::Axis::Z, enabled); }
static void get_x_axis_minimum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool &outEnabled) { outEnabled = component.IsMinimumEnabled(pragma::Axis::X); }
static void get_y_axis_minimum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool &outEnabled) { outEnabled = component.IsMinimumEnabled(pragma::Axis::Y); }
static void get_z_axis_minimum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool &outEnabled) { outEnabled = component.IsMinimumEnabled(pragma::Axis::Z); }

static void set_x_axis_maximum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float limit) { component.SetMaximum(pragma::Axis::X, limit); }
static void set_y_axis_maximum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float limit) { component.SetMaximum(pragma::Axis::Y, limit); }
static void set_z_axis_maximum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float limit) { component.SetMaximum(pragma::Axis::Z, limit); }
static void get_x_axis_maximum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float &outValue) { outValue = component.GetMaximum(pragma::Axis::X); }
static void get_y_axis_maximum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float &outValue) { outValue = component.GetMaximum(pragma::Axis::Y); }
static void get_z_axis_maximum(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, float &outValue) { outValue = component.GetMaximum(pragma::Axis::Z); }

static void set_x_axis_maximum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool enabled) { component.SetMaximumEnabled(pragma::Axis::X, enabled); }
static void set_y_axis_maximum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool enabled) { component.SetMaximumEnabled(pragma::Axis::Y, enabled); }
static void set_z_axis_maximum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool enabled) { component.SetMaximumEnabled(pragma::Axis::Z, enabled); }
static void get_x_axis_maximum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool &outEnabled) { outEnabled = component.IsMaximumEnabled(pragma::Axis::X); }
static void get_y_axis_maximum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool &outEnabled) { outEnabled = component.IsMaximumEnabled(pragma::Axis::Y); }
static void get_z_axis_maximum_enabled(const ComponentMemberInfo &info, ConstraintLimitLocationComponent &component, bool &outEnabled) { outEnabled = component.IsMaximumEnabled(pragma::Axis::Z); }
void ConstraintLimitLocationComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintLimitLocationComponent;

	using TAxisLimit = float;
	using TAxisEnabled = bool;
	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_x_axis_minimum, &get_x_axis_minimum>("minimumX", 0.f);
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "minimumXEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_x_axis_minimum_enabled, &get_x_axis_minimum_enabled>("minimumXEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "minimumX";
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_y_axis_minimum, &get_y_axis_minimum>("minimumY", 0.f);
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "minimumYEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_y_axis_minimum_enabled, &get_y_axis_minimum_enabled>("minimumYEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "minimumY";
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_z_axis_minimum, &get_z_axis_minimum>("minimumZ", 0.f);
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "minimumZEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_z_axis_minimum_enabled, &get_z_axis_minimum_enabled>("minimumZEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "minimumZ";
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_x_axis_maximum, &get_x_axis_maximum>("maximumX", 0.f);
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "maximumXEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_x_axis_maximum_enabled, &get_x_axis_maximum_enabled>("maximumXEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "maximumX";
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_y_axis_maximum, &get_y_axis_maximum>("maximumY", 0.f);
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "maximumYEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_y_axis_maximum_enabled, &get_y_axis_maximum_enabled>("maximumYEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "maximumY";
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_z_axis_maximum, &get_z_axis_maximum>("maximumZ", 0.f);
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "maximumZEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_z_axis_maximum_enabled, &get_z_axis_maximum_enabled>("maximumZEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "maximumZ";
		registerMember(std::move(memberInfo));
	}
}

ConstraintLimitLocationComponent::ConstraintLimitLocationComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintLimitLocationComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(constraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintLimitLocationComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintLimitLocationComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent)) {
		m_constraintC = component.GetHandle<ConstraintComponent>();
		m_constraintC->SetDriverEnabled(false);
	}
}
void ConstraintLimitLocationComponent::SetMinimum(pragma::Axis axis, float value) { m_minimum[umath::to_integral(axis)] = value; }
void ConstraintLimitLocationComponent::SetMaximum(pragma::Axis axis, float value) { m_maximum[umath::to_integral(axis)] = value; }

float ConstraintLimitLocationComponent::GetMinimum(pragma::Axis axis) const { return m_minimum[umath::to_integral(axis)]; }
float ConstraintLimitLocationComponent::GetMaximum(pragma::Axis axis) const { return m_maximum[umath::to_integral(axis)]; }

void ConstraintLimitLocationComponent::SetMinimumEnabled(pragma::Axis axis, bool enabled) { m_minimumEnabled[umath::to_integral(axis)] = enabled; }
bool ConstraintLimitLocationComponent::IsMinimumEnabled(pragma::Axis axis) const { return m_minimumEnabled[umath::to_integral(axis)]; }

void ConstraintLimitLocationComponent::SetMaximumEnabled(pragma::Axis axis, bool enabled) { m_maximumEnabled[umath::to_integral(axis)] = enabled; }
bool ConstraintLimitLocationComponent::IsMaximumEnabled(pragma::Axis axis) const { return m_maximumEnabled[umath::to_integral(axis)]; }
void ConstraintLimitLocationComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;
	Vector3 pos;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberPos(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), pos);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}
	auto origPos = pos;
	constexpr auto numAxes = umath::to_integral(pragma::Axis::Count);
	for(auto i = decltype(numAxes) {0u}; i < numAxes; ++i) {
		auto axis = static_cast<pragma::Axis>(i);
		if(IsMinimumEnabled(axis))
			pos[i] = umath::max(pos[i], GetMinimum(axis));
		if(IsMaximumEnabled(axis))
			pos[i] = umath::min(pos[i], GetMaximum(axis));
	}

	pos = uvec::lerp(origPos, pos, influence);
	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberPos(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), pos);
}
