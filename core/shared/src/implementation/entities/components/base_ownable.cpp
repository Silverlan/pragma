// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/baseentity_events.hpp"

export module pragma.shared;

import :entities.components.base_ownable;

using namespace pragma;

ComponentEventId BaseOwnableComponent::EVENT_ON_OWNER_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseOwnableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_OWNER_CHANGED = registerEvent("ON_OWNER_CHANGED", ComponentEventInfo::Type::Broadcast); }
BaseOwnableComponent::BaseOwnableComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_owner(pragma::EntityProperty::Create()) {}
void BaseOwnableComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	m_netEvSetOwner = SetupNetEvent("set_owner");
}
const pragma::PEntityProperty &BaseOwnableComponent::GetOwnerProperty() const { return m_owner; }
void BaseOwnableComponent::SetOwner(BaseEntity &owner) { SetOwner(&owner); }
void BaseOwnableComponent::ClearOwner() { SetOwner(nullptr); }
void BaseOwnableComponent::SetOwner(BaseEntity *owner)
{
	if(owner == m_owner->GetValue().get())
		return;
	auto *oldOwner = m_owner->GetValue().get();
	*m_owner = (owner != nullptr) ? owner->GetHandle() : EntityHandle {};
	CEOnOwnerChanged evData {oldOwner, m_owner->GetValue().get()};
	BroadcastEvent(EVENT_ON_OWNER_CHANGED, evData);
}
BaseEntity *BaseOwnableComponent::GetOwner() { return (*m_owner)->get(); }
const BaseEntity *BaseOwnableComponent::GetOwner() const { return const_cast<BaseOwnableComponent *>(this)->GetOwner(); }
