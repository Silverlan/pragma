/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraint_manager_component.hpp"
#include "pragma/entities/components/constraint_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/component_member_flags.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

ComponentEventId ConstraintManagerComponent::EVENT_APPLY_CONSTRAINT = pragma::INVALID_COMPONENT_ID;
void ConstraintManagerComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_APPLY_CONSTRAINT = registerEvent("APPLY_CONSTRAINT", ComponentEventInfo::Type::Explicit); }
ConstraintManagerComponent::ConstraintManagerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintManagerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ConstraintManagerComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void ConstraintManagerComponent::ApplyConstraints()
{
	for(auto &cData : m_constraints)
		cData.constraint->ApplyConstraint();
}
void ConstraintManagerComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }
void ConstraintManagerComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	Clear();
}
void ConstraintManagerComponent::Clear() { m_constraints.clear(); }
void ConstraintManagerComponent::ChangeOrder(ConstraintComponent &constraint, int32_t newOrderIndex)
{
	auto curIt = FindConstraint(constraint);
	assert(curIt != m_constraints.end());
	if(curIt == m_constraints.end()) {
		spdlog::warn("Attempted to change constraint order for constraint '{}' in constraint manager '{}', but constraint was not registered with manager.", constraint.GetEntity().ToString(), GetEntity().ToString());
		return;
	}
	auto targetIt = std::lower_bound(m_constraints.begin(), m_constraints.end(), newOrderIndex, [](const ConstraintInfo &info, int32_t idx) { return info->GetOrderIndex() < idx; });
	constraint.m_orderIndex = newOrderIndex;
	// Only re-sort the affected range
	if(curIt < targetIt)
		std::sort(curIt, targetIt, [](const ConstraintInfo &a, const ConstraintInfo &b) { return a.constraint->GetOrderIndex() < b.constraint->GetOrderIndex(); });
	else
		std::sort(targetIt, curIt + 1, [](const ConstraintInfo &a, const ConstraintInfo &b) { return a.constraint->GetOrderIndex() < b.constraint->GetOrderIndex(); });
	// TODO: There's probably a more efficient way to re-sort the array using std::rotate
}
void ConstraintManagerComponent::AddConstraint(ConstraintComponent &constraint)
{
	assert(FindConstraint(constraint) == m_constraints.end());
	util::insert_sorted(m_constraints, ConstraintInfo {&constraint}, [](const ConstraintInfo &a, const ConstraintInfo &b) { return a->GetOrderIndex() < b->GetOrderIndex(); });
}
std::vector<ConstraintManagerComponent::ConstraintInfo>::iterator ConstraintManagerComponent::FindConstraint(ConstraintComponent &constraint)
{
	auto it = std::lower_bound(m_constraints.begin(), m_constraints.end(), constraint.GetOrderIndex(), [](const ConstraintInfo &info, int32_t idx) { return info->GetOrderIndex() < idx; });
	if(it == m_constraints.end())
		return m_constraints.end();
	for(auto i = it; i != m_constraints.end(); ++i) {
		if((*i)->GetOrderIndex() > constraint.GetOrderIndex())
			break;
		if(**i == &constraint)
			return i;
	}
	return m_constraints.end();
}
void ConstraintManagerComponent::RemoveConstraint(ConstraintComponent &constraint)
{
	if(GetEntity().IsRemoved()) {
		Clear();
		return;
	}
	auto it = FindConstraint(constraint);
	if(it == m_constraints.end())
		return;
	m_constraints.erase(it);
}
