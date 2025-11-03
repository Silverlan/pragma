// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/core.hpp"

#include "pragma/logging.hpp"

module pragma.shared;

import :entities.components.constraints.limit_rotation;

using namespace pragma;
static void set_x_limit(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, const Vector2 &limit) { component.SetLimit(pragma::Axis::X, limit); }
static void set_y_limit(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, const Vector2 &limit) { component.SetLimit(pragma::Axis::Y, limit); }
static void set_z_limit(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, const Vector2 &limit) { component.SetLimit(pragma::Axis::Z, limit); }
static void get_x_limit(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, Vector2 &outValue) { outValue = component.GetLimit(pragma::Axis::X); }
static void get_y_limit(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, Vector2 &outValue) { outValue = component.GetLimit(pragma::Axis::Y); }
static void get_z_limit(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, Vector2 &outValue) { outValue = component.GetLimit(pragma::Axis::Z); }

static void set_x_limit_enabled(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, bool enabled) { component.SetLimitEnabled(pragma::Axis::X, enabled); }
static void set_y_limit_enabled(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, bool enabled) { component.SetLimitEnabled(pragma::Axis::Y, enabled); }
static void set_z_limit_enabled(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, bool enabled) { component.SetLimitEnabled(pragma::Axis::Z, enabled); }
static void get_x_limit_enabled(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, bool &outEnabled) { outEnabled = component.IsLimitEnabled(pragma::Axis::X); }
static void get_y_limit_enabled(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, bool &outEnabled) { outEnabled = component.IsLimitEnabled(pragma::Axis::Y); }
static void get_z_limit_enabled(const ComponentMemberInfo &info, ConstraintLimitRotationComponent &component, bool &outEnabled) { outEnabled = component.IsLimitEnabled(pragma::Axis::Z); }
void ConstraintLimitRotationComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintLimitRotationComponent;

	using TAxisLimit = Vector2;
	using TAxisEnabled = bool;
	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_x_limit, &get_x_limit>("limitX", Vector2 {});
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "limitXEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Angle);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_x_limit_enabled, &get_x_limit_enabled>("limitXEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "limitX";
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_y_limit, &get_y_limit>("limitY", Vector2 {});
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "limitYEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Angle);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_y_limit_enabled, &get_y_limit_enabled>("limitYEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "limitY";
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, TAxisLimit, &set_z_limit, &get_z_limit>("limitZ", Vector2 {});
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = "limitZEnabled";
		memberInfo.SetSpecializationType(pragma::AttributeSpecializationType::Angle);
		registerMember(std::move(memberInfo));
	}
	{
		auto memberInfo = create_component_member_info<T, TAxisEnabled, &set_z_limit_enabled, &get_z_limit_enabled>("limitZEnabled", false);
		memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = "limitZ";
		registerMember(std::move(memberInfo));
	}
}

ConstraintLimitRotationComponent::ConstraintLimitRotationComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintLimitRotationComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(constraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintLimitRotationComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintLimitRotationComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent)) {
		m_constraintC = component.GetHandle<ConstraintComponent>();
		m_constraintC->SetDriverEnabled(false);
	}
}
void ConstraintLimitRotationComponent::SetLimit(pragma::Axis axis, const Vector2 &limit) { m_limits[umath::to_integral(axis)] = limit; }
const Vector2 &ConstraintLimitRotationComponent::GetLimit(pragma::Axis axis) const { return m_limits[umath::to_integral(axis)]; }

void ConstraintLimitRotationComponent::SetLimitEnabled(pragma::Axis axis, bool enabled) { m_limitsEnabled[umath::to_integral(axis)] = enabled; }
bool ConstraintLimitRotationComponent::IsLimitEnabled(pragma::Axis axis) const { return m_limitsEnabled[umath::to_integral(axis)]; }
void ConstraintLimitRotationComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;
	Quat rot;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberRot(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rot);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}
	auto ang = EulerAngles {rot};
	auto origRot = rot;
	constexpr auto numAxes = umath::to_integral(pragma::Axis::Count);
	for(auto i = decltype(numAxes) {0u}; i < numAxes; ++i) {
		auto axis = static_cast<pragma::Axis>(i);
		if(!IsLimitEnabled(axis))
			continue;
		auto limit = GetLimit(axis);
		ang[i] = umath::clamp(ang[i], limit.x, limit.y);
	}
	rot = uquat::create(ang);

	rot = uquat::slerp(origRot, rot, influence);
	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberRot(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rot);
}
