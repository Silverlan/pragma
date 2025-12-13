// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.constraints.copy_rotation;

using namespace pragma;

ConstraintCopyRotationComponent::ConstraintCopyRotationComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintCopyRotationComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintSpaceComponent>();
	BindEventUnhandled(constraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintCopyRotationComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintCopyRotationComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
	else if(typeid(component) == typeid(ConstraintSpaceComponent))
		m_constraintSpaceC = component.GetHandle<ConstraintSpaceComponent>();
}
void ConstraintCopyRotationComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;
	Quat rotDriven;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberRot(constraintInfo->drivenObjectPropIdx, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rotDriven);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}

	Quat rotDriver;
	res = constraintInfo->driverC->GetTransformMemberRot(constraintInfo->driverPropIdx, static_cast<math::CoordinateSpace>(m_constraintC->GetDriverSpace()), rotDriver);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driver of constraint '{}'.", constraintInfo->driverPropIdx, GetEntity().ToString());
		return;
	}

	if(m_constraintSpaceC.valid()) {
		auto angDriver = EulerAngles {rotDriver};
		auto angDriven = EulerAngles {rotDriven};
		m_constraintSpaceC->ApplyFilter(angDriver, angDriven, angDriver);
		rotDriver = uquat::create(angDriver);
	}

	rotDriver = uquat::slerp(rotDriven, rotDriver, influence);
	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberRot(constraintInfo->drivenObjectPropIdx, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rotDriver);
}
