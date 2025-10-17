// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.shared;

import :entities.components.constraints.space;

using namespace pragma;
static void set_x_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool enabled) { component.SetAxisEnabled(pragma::Axis::X, enabled); }
static void set_y_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool enabled) { component.SetAxisEnabled(pragma::Axis::Y, enabled); }
static void set_z_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool enabled) { component.SetAxisEnabled(pragma::Axis::Z, enabled); }
static void is_x_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisEnabled(pragma::Axis::X); }
static void is_y_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisEnabled(pragma::Axis::Y); }
static void is_z_axis_enabled(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisEnabled(pragma::Axis::Z); }

static void set_x_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool inverted) { component.SetAxisInverted(pragma::Axis::X, inverted); }
static void set_y_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool inverted) { component.SetAxisInverted(pragma::Axis::Y, inverted); }
static void set_z_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool inverted) { component.SetAxisInverted(pragma::Axis::Z, inverted); }
static void is_x_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisInverted(pragma::Axis::X); }
static void is_y_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisInverted(pragma::Axis::Y); }
static void is_z_axis_inverted(const ComponentMemberInfo &info, ConstraintSpaceComponent &component, bool &outValue) { outValue = component.IsAxisInverted(pragma::Axis::Z); }
void ConstraintSpaceComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
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
ConstraintSpaceComponent::ConstraintSpaceComponent(BaseEntity &ent) : BaseEntityComponent(ent)
{
	m_axisEnabled.fill(true);
	m_axisInverted.fill(false);
}
void ConstraintSpaceComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent<ConstraintComponent>();
}
void ConstraintSpaceComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void ConstraintSpaceComponent::SetAxisEnabled(pragma::Axis axis, bool enabled) { m_axisEnabled[umath::to_integral(axis)] = enabled; }
bool ConstraintSpaceComponent::IsAxisEnabled(pragma::Axis axis) const { return m_axisEnabled[umath::to_integral(axis)]; }

void ConstraintSpaceComponent::SetAxisInverted(pragma::Axis axis, bool inverted) { m_axisInverted[umath::to_integral(axis)] = inverted; }
bool ConstraintSpaceComponent::IsAxisInverted(pragma::Axis axis) const { return m_axisInverted[umath::to_integral(axis)]; }

void ConstraintSpaceComponent::ApplyFilter(const Vector3 &posDriver, const Vector3 &posDriven, Vector3 &outValue) const
{
	constexpr auto numAxes = umath::to_integral(pragma::Axis::Count);
	for(auto i = decltype(numAxes) {0u}; i < numAxes; ++i) {
		if(IsAxisEnabled(static_cast<pragma::Axis>(i)) == false)
			outValue[i] = posDriven[i];
		else if(IsAxisInverted(static_cast<pragma::Axis>(i)))
			outValue[i] = -posDriver[i];
		else
			outValue[i] = posDriver[i];
	}
}
void ConstraintSpaceComponent::ApplyFilter(const EulerAngles &angDriver, const EulerAngles &angDriven, EulerAngles &outValue) const
{
	constexpr auto numAxes = umath::to_integral(pragma::Axis::Count);
	for(auto i = decltype(numAxes) {0u}; i < numAxes; ++i) {
		if(IsAxisEnabled(static_cast<pragma::Axis>(i)) == false)
			outValue[i] = angDriven[i];
		else if(IsAxisInverted(static_cast<pragma::Axis>(i)))
			outValue[i] = -angDriver[i];
		else
			outValue[i] = angDriver[i];
	}
}
