/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_copy_location_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/components/constraints/constraint_space_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

ConstraintCopyLocationComponent::ConstraintCopyLocationComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintCopyLocationComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintSpaceComponent>();
	BindEventUnhandled(ConstraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintCopyLocationComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
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
	auto constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo)
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

	if(m_constraintSpaceC.valid())
		m_constraintSpaceC->ApplyFilter(posDriver, posDriven, posDriver);

	constraintInfo->drivenObjectC->SetTransformMemberPos(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriver, true);
}
