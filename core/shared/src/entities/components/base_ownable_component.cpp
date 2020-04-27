/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

ComponentEventId BaseOwnableComponent::EVENT_ON_OWNER_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseOwnableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_OWNER_CHANGED = componentManager.RegisterEvent("ON_OWNER_CHANGED");
}
BaseOwnableComponent::BaseOwnableComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_owner(pragma::EntityProperty::Create())
{}
void BaseOwnableComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	m_netEvSetOwner = SetupNetEvent("set_owner");
}
const pragma::PEntityProperty &BaseOwnableComponent::GetOwnerProperty() const {return m_owner;}
void BaseOwnableComponent::SetOwner(BaseEntity &owner) {SetOwner(&owner);}
void BaseOwnableComponent::ClearOwner() {SetOwner(nullptr);}
void BaseOwnableComponent::SetOwner(BaseEntity *owner)
{
	if(owner == m_owner->GetValue().get())
		return;
	auto *oldOwner = m_owner->GetValue().get();
	*m_owner = (owner != nullptr) ? owner->GetHandle() : EntityHandle{};
	CEOnOwnerChanged evData {oldOwner,m_owner->GetValue().get()};
	BroadcastEvent(EVENT_ON_OWNER_CHANGED,evData);
}
BaseEntity *BaseOwnableComponent::GetOwner() {return (*m_owner)->get();}
const BaseEntity *BaseOwnableComponent::GetOwner() const {return const_cast<BaseOwnableComponent*>(this)->GetOwner();}
