// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_ownable;

using namespace pragma;

void BaseOwnableComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseOwnableComponent::EVENT_ON_OWNER_CHANGED = registerEvent("ON_OWNER_CHANGED", ComponentEventInfo::Type::Broadcast); }
BaseOwnableComponent::BaseOwnableComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_owner(EntityProperty::Create()) {}
void BaseOwnableComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	m_netEvSetOwner = SetupNetEvent("set_owner");
}
const PEntityProperty &BaseOwnableComponent::GetOwnerProperty() const { return m_owner; }
void BaseOwnableComponent::SetOwner(ecs::BaseEntity &owner) { SetOwner(&owner); }
void BaseOwnableComponent::ClearOwner() { SetOwner(nullptr); }
void BaseOwnableComponent::SetOwner(ecs::BaseEntity *owner)
{
	if(owner == m_owner->GetValue().get())
		return;
	auto *oldOwner = m_owner->GetValue().get();
	*m_owner = (owner != nullptr) ? owner->GetHandle() : EntityHandle {};
	CEOnOwnerChanged evData {oldOwner, m_owner->GetValue().get()};
	BroadcastEvent(baseOwnableComponent::EVENT_ON_OWNER_CHANGED, evData);
}
ecs::BaseEntity *BaseOwnableComponent::GetOwner() { return (*m_owner)->get(); }
const ecs::BaseEntity *BaseOwnableComponent::GetOwner() const { return const_cast<BaseOwnableComponent *>(this)->GetOwner(); }
