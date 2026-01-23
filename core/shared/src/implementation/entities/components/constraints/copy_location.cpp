// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.constraints.copy_location;

using namespace pragma;

ConstraintCopyLocationComponent::ConstraintCopyLocationComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintCopyLocationComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintSpaceComponent>();
	BindEventUnhandled(constraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintCopyLocationComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintCopyLocationComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
	else if(typeid(component) == typeid(ConstraintSpaceComponent))
		m_constraintSpaceC = component.GetHandle<ConstraintSpaceComponent>();
}
void ConstraintCopyLocationComponent::ApplyConstraint()
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

	if(m_constraintSpaceC.valid())
		m_constraintSpaceC->ApplyFilter(posDriver, posDriven, posDriver);

	posDriver = uvec::lerp(posDriven, posDriver, influence);
	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberPos(constraintInfo->drivenObjectPropIdx, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriver);
}
