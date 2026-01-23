// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.components.constraints.manager;

using namespace pragma;

static std::vector<ConstraintManagerComponent::ConstraintInfo> &get_constraints(const NetworkState &nw)
{
	static std::vector<ConstraintManagerComponent::ConstraintInfo> g_sv;
	static std::vector<ConstraintManagerComponent::ConstraintInfo> g_cl;
	return nw.IsServer() ? g_sv : g_cl;
}
void ConstraintManagerComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { constraintManagerComponent::EVENT_APPLY_CONSTRAINT = registerEvent("APPLY_CONSTRAINT", ComponentEventInfo::Type::Explicit); }
ConstraintManagerComponent::ConstraintManagerComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintManagerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ConstraintManagerComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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
