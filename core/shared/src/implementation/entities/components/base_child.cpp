// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_child;

using namespace pragma;

void BaseChildComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseChildComponent::EVENT_ON_PARENT_CHANGED = registerEvent("ON_PARENT_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseChildComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseChildComponent;

	{
		using TParent = EntityURef;
		auto memberInfo = create_component_member_info<T, TParent, static_cast<void (T::*)(const TParent &)>(&T::SetParent), static_cast<const TParent &(T::*)() const>(&T::GetParent)>("parent", TParent {});
		registerMember(std::move(memberInfo));
	}
}
BaseChildComponent::BaseChildComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
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
void BaseChildComponent::SetParent(const EntityURef &parent)
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
	BroadcastEvent(baseChildComponent::EVENT_ON_PARENT_CHANGED);
}
const EntityURef &BaseChildComponent::GetParent() const { return m_parent; }
ecs::BaseEntity *BaseChildComponent::GetParentEntity() { return m_parent.GetEntity(GetGame()); }
bool BaseChildComponent::HasParent() const { return GetParentEntity() != nullptr; }
