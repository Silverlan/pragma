/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_manager_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/component_member_flags.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

static std::vector<ConstraintManagerComponent::ConstraintInfo> &get_constraints(const NetworkState &nw)
{
	static std::vector<ConstraintManagerComponent::ConstraintInfo> g_sv;
	static std::vector<ConstraintManagerComponent::ConstraintInfo> g_cl;
	return nw.IsServer() ? g_sv : g_cl;
}
ComponentEventId ConstraintManagerComponent::EVENT_APPLY_CONSTRAINT = pragma::INVALID_COMPONENT_ID;
void ConstraintManagerComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_APPLY_CONSTRAINT = registerEvent("APPLY_CONSTRAINT", ComponentEventInfo::Type::Explicit); }
ConstraintManagerComponent::ConstraintManagerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintManagerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ConstraintManagerComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

std::vector<ConstraintManagerComponent::ConstraintInfo> &ConstraintManagerComponent::GetConstraints() { return get_constraints(GetNetworkState()); }
void ConstraintManagerComponent::ApplyConstraints(const NetworkState &nw)
{
	for(auto &cData : get_constraints(nw))
		cData.constraint->ApplyConstraint();
}
void ConstraintManagerComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }
void ConstraintManagerComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	Clear();
}
void ConstraintManagerComponent::Clear()
{
	if(m_ownConstraints.empty())
		return;
	auto constraints = std::move(m_ownConstraints);
	m_ownConstraints.clear();
	for(auto *c : constraints)
		ReleaseConstraint(*c);
}
void ConstraintManagerComponent::ChangeOrder(ConstraintComponent &constraint, int32_t newOrderIndex)
{
	auto &constraints = GetConstraints();
	auto curIt = FindConstraint(constraint);
	assert(curIt != constraints.end());
	if(curIt == constraints.end()) {
		spdlog::warn("Attempted to change constraint order for constraint '{}' in constraint manager '{}', but constraint was not registered with manager.", constraint.GetEntity().ToString(), GetEntity().ToString());
		return;
	}
	auto targetIt = std::lower_bound(constraints.begin(), constraints.end(), newOrderIndex, [](const ConstraintInfo &info, int32_t idx) { return info->GetOrderIndex() < idx; });
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
	auto &constraints = GetConstraints();
	assert(FindConstraint(constraint) == constraints.end());
	util::insert_sorted(constraints, ConstraintInfo {&constraint}, [](const ConstraintInfo &a, const ConstraintInfo &b) { return a->GetOrderIndex() < b->GetOrderIndex(); });
	m_ownConstraints.push_back(&constraint);
}
std::vector<ConstraintManagerComponent::ConstraintInfo>::iterator ConstraintManagerComponent::FindConstraint(ConstraintComponent &constraint)
{
	auto &constraints = GetConstraints();
	auto it = std::lower_bound(constraints.begin(), constraints.end(), constraint.GetOrderIndex(), [](const ConstraintInfo &info, int32_t idx) { return info->GetOrderIndex() < idx; });
	if(it == constraints.end())
		return constraints.end();
	for(auto i = it; i != constraints.end(); ++i) {
		if((*i)->GetOrderIndex() > constraint.GetOrderIndex())
			break;
		if(**i == &constraint)
			return i;
	}
	return constraints.end();
}
void ConstraintManagerComponent::ReleaseConstraint(ConstraintComponent &constraint)
{
	auto &constraints = GetConstraints();
	auto it = FindConstraint(constraint);
	if(it == constraints.end())
		return;
	constraints.erase(it);
	auto itOwn = std::find(m_ownConstraints.begin(), m_ownConstraints.end(), &constraint);
	if(itOwn != m_ownConstraints.end())
		m_ownConstraints.erase(itOwn);
}
void ConstraintManagerComponent::RemoveConstraint(ConstraintComponent &constraint)
{
	if(GetEntity().IsRemoved()) {
		Clear();
		return;
	}
	ReleaseConstraint(constraint);
}
