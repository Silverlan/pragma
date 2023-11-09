/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_copy_scale_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/components/constraints/constraint_space_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

ConstraintCopyScaleComponent::ConstraintCopyScaleComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintCopyScaleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintSpaceComponent>();
	BindEventUnhandled(ConstraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
}
void ConstraintCopyScaleComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintCopyScaleComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
	else if(typeid(component) == typeid(ConstraintSpaceComponent))
		m_constraintSpaceC = component.GetHandle<ConstraintSpaceComponent>();
}
void ConstraintCopyScaleComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;
	Vector3 scaleDriven;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberScale(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), scaleDriven);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}

	Vector3 scaleDriver;
	res = constraintInfo->driverC->GetTransformMemberScale(constraintInfo->driverPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDriverSpace()), scaleDriver);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driver of constraint '{}'.", constraintInfo->driverPropIdx, GetEntity().ToString());
		return;
	}

	if(m_constraintSpaceC.valid())
		m_constraintSpaceC->ApplyFilter(scaleDriver, scaleDriven, scaleDriver);

	scaleDriver = uvec::lerp(scaleDriven, scaleDriver, influence);
	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberScale(constraintInfo->drivenObjectPropIdx, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), scaleDriver);
}
