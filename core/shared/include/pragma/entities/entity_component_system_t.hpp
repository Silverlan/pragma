/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_COMPONENT_SYSTEM_T_HPP__
#define __ENTITY_COMPONENT_SYSTEM_T_HPP__

#include "pragma/entities/entity_component_system.hpp"
#include "pragma/entities/components/base_entity_component.hpp"

template<class TComponent, typename>
pragma::ComponentHandle<TComponent> pragma::BaseEntityComponentSystem::AddComponent(bool bForceCreateNew)
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
		return {};
	auto whComponent = AddComponent(componentId, bForceCreateNew);
	if(whComponent.expired())
		return {};
	return whComponent->GetHandle<TComponent>();
}
template<class TComponent, typename>
void pragma::BaseEntityComponentSystem::RemoveComponent()
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
		return;
	RemoveComponent(componentId);
}
template<class TComponent, typename>
pragma::ComponentHandle<TComponent> pragma::BaseEntityComponentSystem::GetComponent() const
{
	ComponentId componentId;
	if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)), componentId) == false)
		return pragma::ComponentHandle<TComponent> {};
	auto it = m_componentLookupTable.find(componentId);
	return (it != m_componentLookupTable.end()) ? const_cast<BaseEntityComponent *>(it->second.get())->GetHandle<TComponent>() : pragma::ComponentHandle<TComponent> {};
}
template<class TComponent, typename>
bool pragma::BaseEntityComponentSystem::HasComponent() const
{
	ComponentId componentId;
	if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)), componentId) == false)
		return false;
	auto it = m_componentLookupTable.find(componentId);
	return it != m_componentLookupTable.end() && it->second.expired() == false;
}

#endif
