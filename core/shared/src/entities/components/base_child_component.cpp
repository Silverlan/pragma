/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_child_component.hpp"
#include "pragma/entities/components/parent_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

ComponentEventId BaseChildComponent::EVENT_ON_PARENT_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseChildComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_PARENT_CHANGED = registerEvent("ON_PARENT_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseChildComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseChildComponent;

	{
		using TParent = pragma::EntityURef;
		auto memberInfo = create_component_member_info<T, TParent, static_cast<void (T::*)(const TParent &)>(&T::SetParent), static_cast<const TParent &(T::*)() const>(&T::GetParent)>("parent", TParent {});
		registerMember(std::move(memberInfo));
	}
}
BaseChildComponent::BaseChildComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseChildComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetParent = SetupNetEvent("set_parent");
}
void BaseChildComponent::OnRemove() { BaseEntityComponent::OnRemove(); }
void BaseChildComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(!m_parentValid)
		SetParent(m_parent); // Try again if parent wasn't valid before
}
void BaseChildComponent::ClearParent()
{
	m_parentValid = false;
	auto *entParent = GetParentEntity();
	if(entParent) {
		auto parentC = entParent->GetComponent<ParentComponent>();
		if(parentC.valid())
			parentC->RemoveChild(*this);
	}
}
void BaseChildComponent::SetParent(const pragma::EntityURef &parent)
{
	ClearParent();
	m_parent = parent;
	auto *entParent = GetParentEntity();
	if(entParent) {
		m_parentValid = true;
		if(entParent == &GetEntity())
			throw std::logic_error {"Attempted to assign entity as parent to itself! This is not allowed!"};
		auto parentC = entParent->AddComponent<ParentComponent>();
		if(parentC.valid())
			parentC->AddChild(*this);
	}
	else
		m_parentValid = false;

	OnParentChanged(entParent);
	BroadcastEvent(EVENT_ON_PARENT_CHANGED);
}
const pragma::EntityURef &BaseChildComponent::GetParent() const { return m_parent; }
BaseEntity *BaseChildComponent::GetParentEntity() { return m_parent.GetEntity(GetGame()); }
bool BaseChildComponent::HasParent() const { return GetParentEntity() != nullptr; }
