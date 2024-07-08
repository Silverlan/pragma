/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/parent_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_child_component.hpp"

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
