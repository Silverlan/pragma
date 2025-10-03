// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "algorithm"

module pragma.shared;

import :entities.components.parent;

using namespace pragma;

ComponentEventId ParentComponent::EVENT_ON_CHILD_ADDED = pragma::INVALID_COMPONENT_ID;
ComponentEventId ParentComponent::EVENT_ON_CHILD_REMOVED = pragma::INVALID_COMPONENT_ID;
void ParentComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_CHILD_ADDED = registerEvent("ON_CHILD_ADDED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_CHILD_REMOVED = registerEvent("ON_CHILD_REMOVED", ComponentEventInfo::Type::Broadcast);
}
ParentComponent::ParentComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ParentComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ParentComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ParentComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	for(auto &hChild : m_children) {
		if(hChild.expired())
			continue;
		hChild->GetEntity().Remove();
	}
}
const std::vector<util::TWeakSharedHandle<BaseChildComponent>> &ParentComponent::GetChildren() const { return const_cast<ParentComponent *>(this)->GetChildren(); }
std::vector<util::TWeakSharedHandle<BaseChildComponent>> &ParentComponent::GetChildren() { return m_children; }
void ParentComponent::RemoveChild(BaseChildComponent &child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(), [&child](const ComponentHandle<BaseChildComponent> &component) { return &child == component.get(); });
	if(it == m_children.end())
		return;
	m_children.erase(it);
	BroadcastEvent(EVENT_ON_CHILD_REMOVED, CEOnChildRemoved {child});
}
void ParentComponent::AddChild(BaseChildComponent &child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(), [&child](const ComponentHandle<BaseChildComponent> &component) { return &child == component.get(); });
	if(it != m_children.end())
		return;
	m_children.push_back(child.GetHandle<BaseChildComponent>());
	BroadcastEvent(EVENT_ON_CHILD_ADDED, CEOnChildAdded {child});
}

////////////

CEOnChildAdded::CEOnChildAdded(BaseChildComponent &child) : child {child} {}
void CEOnChildAdded::PushArguments(lua_State *l) { child.GetLuaObject().push(l); }
