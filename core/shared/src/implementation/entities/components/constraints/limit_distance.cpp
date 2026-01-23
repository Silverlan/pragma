// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.constraints.limit_distance;

using namespace pragma;

void ConstraintLimitDistanceComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintLimitDistanceComponent;

	{
		using TDistance = float;
		auto memberInfo = create_component_member_info<T, TDistance, static_cast<void (T::*)(TDistance)>(&T::SetDistance), static_cast<TDistance (T::*)() const>(&T::GetDistance)>("distance", 0.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetSpecializationType(AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}

	{
		using TClampRegion = ClampRegion;
		auto memberInfo = create_component_member_info<T, TClampRegion, static_cast<void (T::*)(TClampRegion)>(&T::SetClampRegion), static_cast<TClampRegion (T::*)() const>(&T::GetClampRegion)>("clampRegion", TClampRegion::Inside);
		registerMember(std::move(memberInfo));
	}
}

ConstraintLimitDistanceComponent::ConstraintLimitDistanceComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintLimitDistanceComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(constraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintLimitDistanceComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintLimitDistanceComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
}
void ConstraintLimitDistanceComponent::SetClampRegion(ClampRegion clampRegion) { m_clampRegion = clampRegion; }
ConstraintLimitDistanceComponent::ClampRegion ConstraintLimitDistanceComponent::GetClampRegion() const { return m_clampRegion; }

void ConstraintLimitDistanceComponent::SetDistance(float dist)
{
	m_dist = dist;
	m_distSqr = math::pow2(dist);
}
float ConstraintLimitDistanceComponent::GetDistance() const { return m_dist; }
void ConstraintLimitDistanceComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;
	Vector3 posDriven;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberPos(constraintInfo->drivenObjectPropIdx, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriven);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}

	Vector3 posDriver;
	res = constraintInfo->driverC->GetTransformMemberPos(constraintInfo->driverPropIdx, static_cast<math::CoordinateSpace>(m_constraintC->GetDriverSpace()), posDriver);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driver of constraint '{}'.", constraintInfo->driverPropIdx, GetEntity().ToString());
		return;
	}

	auto distSqr = uvec::distance_sqr(posDriver, posDriven);
	auto clampRegion = GetClampRegion();
	switch(clampRegion) {
	case ClampRegion::Inside:
		if(distSqr <= m_distSqr)
			return; // Nothing to do
		break;
	case ClampRegion::Outside:
		if(distSqr >= m_distSqr)
			return; // Nothing to do
		break;
	}
	auto origin = posDriver;
	auto dir = posDriven - origin;
	auto l = uvec::length(dir);
	if(l < 0.0001f)
		dir = uvec::PRM_FORWARD;
	else
		dir /= l;
	posDriver = origin + dir * GetDistance();

	posDriver = uvec::lerp(posDriven, posDriver, influence);
	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberPos(constraintInfo->drivenObjectPropIdx, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriver);
}
