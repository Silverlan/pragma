// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.constraints.space;

using namespace pragma;
static void set_x_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool enabled) { component.SetAxisEnabled(Axis::X, enabled); }
static void set_y_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool enabled) { component.SetAxisEnabled(Axis::Y, enabled); }
static void set_z_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool enabled) { component.SetAxisEnabled(Axis::Z, enabled); }
static void is_x_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisEnabled(Axis::X); }
static void is_y_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisEnabled(Axis::Y); }
static void is_z_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisEnabled(Axis::Z); }

static void set_x_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool inverted) { component.SetAxisInverted(Axis::X, inverted); }
static void set_y_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool inverted) { component.SetAxisInverted(Axis::Y, inverted); }
static void set_z_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool inverted) { component.SetAxisInverted(Axis::Z, inverted); }
static void is_x_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisInverted(Axis::X); }
static void is_y_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisInverted(Axis::Y); }
static void is_z_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisInverted(Axis::Z); }
void ConstraintSpaceComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintSpaceComponent;

	using TAxisEnabled = bool;
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_x_axis_enabled, &is_x_axis_enabled>("xAxisEnabled", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_y_axis_enabled, &is_y_axis_enabled>("yAxisEnabled", true);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_z_axis_enabled, &is_z_axis_enabled>("zAxisEnabled", true);
		registerMember(std::move(memberInfo));
	}

	using TAxisInverted = bool;
	{
		auto memberInfo = create_component_member_info<T, TAxisInverted, &set_x_axis_inverted, &is_x_axis_inverted>("xAxisInverted", false);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisInverted, &set_y_axis_inverted, &is_y_axis_inverted>("yAxisInverted", false);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisInverted, &set_z_axis_inverted, &is_z_axis_inverted>("zAxisInverted", false);
		registerMember(std::move(memberInfo));
	}
}
ConstraintSpaceComponent::ConstraintSpaceComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent)
{
	m_axisEnabled.fill(true);
	m_axisInverted.fill(false);
}
void ConstraintSpaceComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent<ConstraintComponent>();
}
void ConstraintSpaceComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void ConstraintSpaceComponent::SetAxisEnabled(Axis axis, bool enabled) { m_axisEnabled[math::to_integral(axis)] = enabled; }
bool ConstraintSpaceComponent::IsAxisEnabled(Axis axis) const { return m_axisEnabled[math::to_integral(axis)]; }

void ConstraintSpaceComponent::SetAxisInverted(Axis axis, bool inverted) { m_axisInverted[math::to_integral(axis)] = inverted; }
bool ConstraintSpaceComponent::IsAxisInverted(Axis axis) const { return m_axisInverted[math::to_integral(axis)]; }

void ConstraintSpaceComponent::ApplyFilter(const Vector3 &posDriver, const Vector3 &posDriven, Vector3 &outValue) const
{
	constexpr auto numAxes = math::to_integral(Axis::Count);
	for(auto i = decltype(numAxes) {0u}; i < numAxes; ++i) {
		if(IsAxisEnabled(static_cast<Axis>(i)) == false)
			outValue[i] = posDriven[i];
		else if(IsAxisInverted(static_cast<Axis>(i)))
			outValue[i] = -posDriver[i];
		else
			outValue[i] = posDriver[i];
	}
}
void ConstraintSpaceComponent::ApplyFilter(const EulerAngles &angDriver, const EulerAngles &angDriven, EulerAngles &outValue) const
{
	constexpr auto numAxes = math::to_integral(Axis::Count);
	for(auto i = decltype(numAxes) {0u}; i < numAxes; ++i) {
		if(IsAxisEnabled(static_cast<Axis>(i)) == false)
			outValue[i] = angDriven[i];
		else if(IsAxisInverted(static_cast<Axis>(i)))
			outValue[i] = -angDriver[i];
		else
			outValue[i] = angDriver[i];
	}
}
