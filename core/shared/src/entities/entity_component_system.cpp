/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/entity_component_system.hpp"
#include <unordered_set>

using namespace pragma;

BaseEntityComponentSystem::~BaseEntityComponentSystem()
{
	ClearComponents();
}
void BaseEntityComponentSystem::Initialize(BaseEntity &ent,EntityComponentManager &componentManager)
{
	m_entity = &ent;
	m_componentManager = &componentManager;
}
EntityComponentManager *BaseEntityComponentSystem::GetComponentManager() {return m_componentManager;}
const EntityComponentManager *BaseEntityComponentSystem::GetComponentManager() const {return const_cast<BaseEntityComponentSystem*>(this)->GetComponentManager();}
void BaseEntityComponentSystem::ClearComponents()
{
	while(m_components.empty() == false)
	{
		auto numComponents = m_components.size();
		RemoveComponent(m_components.back()->GetComponentId());
		if(m_components.size() == numComponents)
			throw std::logic_error("An unknown error occured when trying to remove an entity component!");
	}
}
util::EventReply BaseEntityComponentSystem::BroadcastEvent(ComponentEventId ev,ComponentEvent &evData,const BaseEntityComponent *src) const
{
	// We need to copy the current list of components, in case one of the events
	// changes the container by adding or removing a component
	auto tmpComponents = m_components;
	for(auto &component : tmpComponents)
	{
		if(component == nullptr || component.get() == src)
			continue;
		if(component->HandleEvent(ev,evData) == util::EventReply::Handled)
			return util::EventReply::Handled;
	}
	return util::EventReply::Unhandled;
}
util::EventReply BaseEntityComponentSystem::BroadcastEvent(ComponentEventId eventId) const
{
	CEGenericComponentEvent ev {};
	return BroadcastEvent(eventId,ev);
}
util::WeakHandle<pragma::BaseEntityComponent> BaseEntityComponentSystem::AddComponent(ComponentId componentId,bool bForceCreateNew)
{
	if(bForceCreateNew == false)
	{
		auto it = std::find_if(m_components.begin(),m_components.end(),[componentId](const std::shared_ptr<pragma::BaseEntityComponent> &ptrComponent) {
			return ptrComponent->GetComponentId() == componentId;
		});
		if(it != m_components.end())
			return *it;
	}
	auto ptrComponent = m_componentManager->CreateComponent(componentId,*m_entity);
	if(ptrComponent == nullptr)
	{
		Con::cwar<<"WARNING: Unable to create entity component of type "<<componentId<<"!"<<this<<Con::endl;
		return {};
	}
	if(m_components.size() == m_components.capacity())
		m_components.reserve(m_components.size() +5u);
	m_components.push_back(ptrComponent);
	auto it = m_componentLookupTable.find(componentId);
	if(it == m_componentLookupTable.end())
		m_componentLookupTable.insert(std::make_pair(componentId,ptrComponent));

	ptrComponent->Initialize();

	// We need a copy of all components that have been created up to this point, in case
	// one of the following functions adds or removes components.
	auto components = m_components;

	OnComponentAdded(*ptrComponent);

	pragma::CEOnEntityComponentAdded evData{*ptrComponent};
	BroadcastEvent(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED,evData,ptrComponent.get());

	for(auto &ptrComponentOther : components)
	{
		if(ptrComponentOther == nullptr || ptrComponentOther.get() == ptrComponent.get())
			continue;
		ptrComponentOther->OnEntityComponentAdded(*ptrComponent,false);
		ptrComponent->OnEntityComponentAdded(*ptrComponentOther,true);
	}
	return ptrComponent;
}
util::WeakHandle<pragma::BaseEntityComponent> BaseEntityComponentSystem::AddComponent(const std::string &name,bool bForceCreateNew)
{
	pragma::ComponentId componentId;
	if(m_componentManager->GetComponentTypeId(name,componentId) == false)
	{
		if(
			// Component isn't registered, try to load it dynamically
			m_entity->GetNetworkState()->GetGameState()->LoadLuaComponentByName(name) == false ||
			m_componentManager->GetComponentTypeId(name,componentId) == false
		)
		{
			Con::cwar<<"WARNING: Attempted to add unknown component '"<<name<<"' to game object "<<this<<Con::endl;
			return {};
		}
	}
	return AddComponent(componentId,bForceCreateNew);
}
void BaseEntityComponentSystem::RemoveComponent(pragma::BaseEntityComponent &component)
{
	auto it = std::find_if(m_components.begin(),m_components.end(),[&component](const std::shared_ptr<BaseEntityComponent> &componentOther) {
		return componentOther.get() == &component;
	});
	if(it == m_components.end())
		return;
	auto componentId = component.GetComponentId();
	for(auto &ptrComponentOther : m_components)
	{
		if(ptrComponentOther.get() == &component)
			continue;
		ptrComponentOther->OnEntityComponentRemoved(component);
	}
	component.OnRemove();
	OnComponentRemoved(component);
	m_components.erase(it);

	auto itType = m_componentLookupTable.find(componentId);
	if(itType != m_componentLookupTable.end())
	{
		// Find a different component of the same type
		auto it = std::find_if(m_components.begin(),m_components.end(),[componentId](const std::shared_ptr<BaseEntityComponent> &ptrComponent) {
			return ptrComponent->GetComponentId() == componentId;
		});
		if(it == m_components.end())
		{
			m_componentLookupTable.erase(itType);
			return;
		}
		itType->second = *it;
	}
}
void BaseEntityComponentSystem::RemoveComponent(ComponentId componentId)
{
	auto hComponent = FindComponent(componentId);
	while(hComponent.valid())
	{
		RemoveComponent(*hComponent);
		hComponent = FindComponent(componentId);
	}
}
void BaseEntityComponentSystem::RemoveComponent(const std::string &name)
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId(name,componentId) == false)
		return;
	RemoveComponent(componentId);
}
void BaseEntityComponentSystem::OnComponentAdded(BaseEntityComponent &component) {}
void BaseEntityComponentSystem::OnComponentRemoved(BaseEntityComponent &component) {}
bool pragma::BaseEntityComponentSystem::HasComponent(ComponentId componentId) const
{
	auto it = m_componentLookupTable.find(componentId);
	return it != m_componentLookupTable.end() && it->second.expired() == false;
}

const std::vector<std::shared_ptr<BaseEntityComponent>> &BaseEntityComponentSystem::GetComponents() const {return const_cast<BaseEntityComponentSystem*>(this)->GetComponents();}
std::vector<std::shared_ptr<BaseEntityComponent>> &BaseEntityComponentSystem::GetComponents() {return m_components;}

util::WeakHandle<BaseEntityComponent> BaseEntityComponentSystem::FindComponent(ComponentId componentId) const
{
	auto it = m_componentLookupTable.find(componentId);
	if(it == m_componentLookupTable.end())
		return {};
	return it->second.lock();
}
util::WeakHandle<BaseEntityComponent> BaseEntityComponentSystem::FindComponent(const std::string &name) const
{
	ComponentId componentId;
	auto typeIndex = std::type_index(typeid(*this));
	if(m_componentManager->GetComponentTypeId(name,componentId) == false)
		return {};
	return FindComponent(componentId);
}
