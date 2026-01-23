// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.system;

using namespace pragma;
static std::vector<BaseEntityComponentSystem *> g_systemsScheduledForCleanup; // TODO: It would be cleaner to have one instance of this per game state
BaseEntityComponentSystem::~BaseEntityComponentSystem()
{
	if(!m_components.empty())
		throw std::runtime_error {"m_components is not empty! Did you forget to call ClearComponents()?"};
	if(math::is_flag_set(m_stateFlags, StateFlags::ComponentCleanupRequired)) {
		auto it = std::find(g_systemsScheduledForCleanup.begin(), g_systemsScheduledForCleanup.end(), this);
		assert(it != g_systemsScheduledForCleanup.end());
		if(it != g_systemsScheduledForCleanup.end())
			g_systemsScheduledForCleanup.erase(it);
	}
}
void BaseEntityComponentSystem::OnRemove() { math::set_flag(m_stateFlags, StateFlags::IsBeingRemoved); }
void BaseEntityComponentSystem::Initialize(ecs::BaseEntity &ent, EntityComponentManager &componentManager)
{
	m_entity = &ent;
	m_componentManager = &componentManager;
}
EntityComponentManager *BaseEntityComponentSystem::GetComponentManager() { return m_componentManager; }
const EntityComponentManager *BaseEntityComponentSystem::GetComponentManager() const { return const_cast<BaseEntityComponentSystem *>(this)->GetComponentManager(); }
void BaseEntityComponentSystem::ClearComponents()
{
	while(!m_components.empty()) {
		for(auto &c : m_components) {
			if(c.expired())
				continue;
			RemoveComponent(c->GetComponentId());
		}
		if(m_componentLookupTable.empty())
			break;
		// The removal of an entity component may have caused another component to be created, so we may have to do
		// multiple iterations to properly remove them all.
		for(auto it = m_components.begin(); it != m_components.end();) {
			auto &c = *it;
			if(c.expired())
				it = m_components.erase(it);
			else
				++it;
		}
	}
	m_components.clear();
}
util::EventReply BaseEntityComponentSystem::BroadcastEvent(ComponentEventId ev, ComponentEvent &evData, const BaseEntityComponent *src) const
{
	// Note: This function must only be called from one thread at a time.
	// For this reason multi-threaded events should never be broadcasted, and should
	// always use InvokeEventCallbacks instead.
	auto &components = const_cast<BaseEntityComponentSystem *>(this)->m_components;

	// During the loop, an event callback may create other components, which would invalidate our iterator.
	// We'll track how many components we have to catch events like that.
	auto numInitialComponents = components.size();
	auto numComponents = numInitialComponents;
	for(auto it = components.begin(); it != components.begin() + numInitialComponents; ++it) {
		auto &component = *it;
		if(component == nullptr || component.get() == src)
			continue;
		auto idx = (it - components.begin());
		if(component->HandleEvent(ev, evData) == util::EventReply::Handled)
			return util::EventReply::Handled;
		if(components.size() != numComponents) {
			// Iterator has been invalidated. This can happen because a new component has been added.
			it = components.begin() + idx;
			numComponents = components.size();
		}
	}
	return util::EventReply::Unhandled;
}
util::EventReply BaseEntityComponentSystem::BroadcastEvent(ComponentEventId eventId) const
{
	CEGenericComponentEvent ev {};
	return BroadcastEvent(eventId, ev);
}
ComponentHandle<BaseEntityComponent> BaseEntityComponentSystem::AddComponent(ComponentId componentId, bool bForceCreateNew)
{
	if(bForceCreateNew == false) {
		auto it = std::find_if(m_components.begin(), m_components.end(), [componentId](const util::TSharedHandle<BaseEntityComponent> &ptrComponent) { return ptrComponent.valid() && ptrComponent->GetComponentId() == componentId; });
		if(it != m_components.end())
			return {*it};
	}
	auto ptrComponent = m_componentManager->CreateComponent(componentId, *m_entity);
	if(ptrComponent == nullptr) {
		Con::CWAR << "Unable to create entity component of type " << componentId << " for system " << this << "!" << Con::endl;
		return {};
	}
	if(m_components.size() == m_components.capacity())
		m_components.reserve(m_components.size() + 5u);
	m_components.push_back(ptrComponent);
	auto it = m_componentLookupTable.find(componentId);
	if(it == m_componentLookupTable.end())
		m_componentLookupTable.insert(std::make_pair(componentId, ptrComponent));

	ptrComponent->m_stateFlags |= BaseEntityComponent::StateFlags::IsInitializing;
	ptrComponent->Initialize();

	// We want to call OnEntityComponentAdded for all components that exist up to this point.
	// Since the functions below may add more components, we'll keep track of how many we have right now.
	auto numCurComponents = m_components.size();

	OnComponentAdded(*ptrComponent);

	CEOnEntityComponentAdded evData {*ptrComponent};
	auto *genericC = m_entity->GetGenericComponent();
	if(BroadcastEvent(baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED, evData, ptrComponent.get()) != util::EventReply::Handled && genericC)
		genericC->InvokeEventCallbacks(baseGenericComponent::EVENT_ON_ENTITY_COMPONENT_ADDED, evData);

	assert(numCurComponents <= m_components.size());
	if(numCurComponents > m_components.size())
		throw std::runtime_error {"m_components has shrunk, which is illegal at this point in time!"};
	for(auto i = decltype(numCurComponents) {0u}; i < numCurComponents; ++i) {
		auto &ptrComponentOther = m_components[i];
		if(ptrComponentOther.expired() || ptrComponentOther.get() == ptrComponent.get())
			continue;
		// TODO: This is quite slow and should be handled through an event in the generic entity component instead
		// (Same for OnEntityComponentRemoved)
		ptrComponentOther->OnEntityComponentAdded(*ptrComponent, false);
		ptrComponent->OnEntityComponentAdded(*ptrComponentOther, true);
	}
	ptrComponent->m_stateFlags &= ~BaseEntityComponent::StateFlags::IsInitializing;
	ptrComponent->PostInitialize();
	return {ptrComponent};
}
ComponentHandle<BaseEntityComponent> BaseEntityComponentSystem::AddComponent(const std::string &name, bool bForceCreateNew)
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId(name, componentId) == false) {
		if(
		  // Component isn't registered, try to load it dynamically
		  m_entity->GetNetworkState()->GetGameState()->LoadLuaComponentByName(name) == false || m_componentManager->GetComponentTypeId(name, componentId) == false) {
			Con::CWAR << "Attempted to add unknown component '" << name << "' to game object " << this << Con::endl;
			return {};
		}
	}
	return AddComponent(componentId, bForceCreateNew);
}
void BaseEntityComponentSystem::Cleanup()
{
	for(auto *sys : g_systemsScheduledForCleanup) {
		for(auto it = sys->m_components.begin(); it != sys->m_components.end();) {
			if(it->valid()) {
				++it;
				continue;
			}
			it = sys->m_components.erase(it);
		}
		math::set_flag(sys->m_stateFlags, StateFlags::ComponentCleanupRequired, false);
	}
	g_systemsScheduledForCleanup.clear();
}
void BaseEntityComponentSystem::RemoveComponent(BaseEntityComponent &component)
{
	if(math::is_flag_set(component.m_stateFlags, BaseEntityComponent::StateFlags::Removed))
		return;
	if(math::is_flag_set(component.m_stateFlags, BaseEntityComponent::StateFlags::IsInitializing))
		throw std::runtime_error {"Attempted to remove component of type " + std::to_string(component.GetComponentId()) + " while it is being initialized. This is not allowed!"};
	math::set_flag(component.m_stateFlags, BaseEntityComponent::StateFlags::Removed);
	auto it = std::find_if(m_components.begin(), m_components.end(), [&component](const util::TSharedHandle<BaseEntityComponent> &componentOther) { return componentOther.valid() && componentOther.get() == &component; });
	if(it == m_components.end())
		return;
	auto idxComponent = (it - m_components.begin());
	auto componentId = component.GetComponentId();
	auto numInitialComponents = m_components.size();
	auto numComponents = numInitialComponents;
	for(auto it = m_components.begin(); it != m_components.begin() + numInitialComponents; ++it) {
		auto &c = *it;
		if(c.expired() || c.get() == &component)
			continue;
		auto idx = (it - m_components.begin());
		c->OnEntityComponentRemoved(component);
		if(m_components.size() != numComponents) {
			// OnEntityComponentRemoved has added new components, we have to update the iterator
			it = m_components.begin() + idx;
			numComponents = m_components.size();
		}
	}
	for(auto &ptrComponentOther : m_components) {
		if(ptrComponentOther.expired() || ptrComponentOther.get() == &component)
			continue;
		ptrComponentOther->OnEntityComponentRemoved(component);
	}

	// The OnEntityComponentRemoved may have modified m_components and invalidated the iterator, so we have to re-grab the iterator
	if(idxComponent >= m_components.size())
		throw std::runtime_error {"Component was erased from component list by callback while it was already being removed, this is not allowed!"};
	it = m_components.begin() + idxComponent;
	if(it == m_components.end() || it->get() != &component) {
		// Unreachable?
		throw std::runtime_error {"Component was erased from component list by callback while it was already being removed, this is not allowed!"};
	}
	//

	// Keep component alive temporarily
	auto tmpHandle = *it;
	// Clear the component. We can't erase it from m_components safely, so we just invalidate it
	// for now. m_components will get cleaned up at a later date
	*it = pragma::util::TSharedHandle<BaseEntityComponent> {};
	if(!math::is_flag_set(m_stateFlags, StateFlags::ComponentCleanupRequired)) {
		if(!math::is_flag_set(m_stateFlags, StateFlags::IsBeingRemoved)) { // No point for cleanup if we're already being removed
			math::set_flag(m_stateFlags, StateFlags::ComponentCleanupRequired, true);
			if(g_systemsScheduledForCleanup.size() == g_systemsScheduledForCleanup.capacity())
				g_systemsScheduledForCleanup.reserve(g_systemsScheduledForCleanup.size() * 1.5 + 100);
			g_systemsScheduledForCleanup.push_back(this);
		}
	}
	auto *reg = dynamic_cast<DynamicMemberRegister *>(&component);
	if(reg) {
		if(!reg->GetMembers().empty()) {
			reg->ClearMembers();
			component.OnMembersChanged();
		}
	}
	component.OnRemove();
	OnComponentRemoved(component);
	component.CleanUp();

	// Safe to free now
	tmpHandle = pragma::util::TSharedHandle<BaseEntityComponent> {};

	auto itType = m_componentLookupTable.find(componentId);
	if(itType != m_componentLookupTable.end()) {
		// Find a different component of the same type
		auto it = std::find_if(m_components.begin(), m_components.end(), [componentId](const util::TSharedHandle<BaseEntityComponent> &ptrComponent) { return ptrComponent.valid() && ptrComponent->GetComponentId() == componentId; });
		if(it == m_components.end()) {
			m_componentLookupTable.erase(itType);
			return;
		}
		itType->second = *it;
	}
}
void BaseEntityComponentSystem::RemoveComponent(ComponentId componentId)
{
	auto hComponent = FindComponent(componentId);
	while(hComponent.valid()) {
		RemoveComponent(*hComponent);
		hComponent = FindComponent(componentId);
	}
}
void BaseEntityComponentSystem::RemoveComponent(const std::string &name)
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId(name, componentId) == false)
		return;
	RemoveComponent(componentId);
}
void BaseEntityComponentSystem::OnComponentAdded(BaseEntityComponent &component) {}
void BaseEntityComponentSystem::OnComponentRemoved(BaseEntityComponent &component) {}
bool BaseEntityComponentSystem::HasComponent(ComponentId componentId) const
{
	auto it = m_componentLookupTable.find(componentId);
	return it != m_componentLookupTable.end() && it->second.expired() == false;
}

const std::vector<util::TSharedHandle<BaseEntityComponent>> &BaseEntityComponentSystem::GetComponents() const { return const_cast<BaseEntityComponentSystem *>(this)->GetComponents(); }
std::vector<util::TSharedHandle<BaseEntityComponent>> &BaseEntityComponentSystem::GetComponents() { return m_components; }

ComponentHandle<BaseEntityComponent> BaseEntityComponentSystem::FindComponent(ComponentId componentId) const
{
	auto it = m_componentLookupTable.find(componentId);
	if(it == m_componentLookupTable.end())
		return {};
	return it->second;
}
ComponentHandle<BaseEntityComponent> BaseEntityComponentSystem::FindComponent(const std::string &name) const
{
	ComponentId componentId;
	auto typeIndex = std::type_index(typeid(*this));
	if(m_componentManager->GetComponentTypeId(name, componentId) == false)
		return {};
	return FindComponent(componentId);
}
