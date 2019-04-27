#ifndef __ENTITY_COMPONENT_SYSTEM_T_HPP__
#define __ENTITY_COMPONENT_SYSTEM_T_HPP__

#include "pragma/entities/entity_component_system.hpp"
#include "pragma/entities/components/base_entity_component.hpp"

template<class TComponent,typename>
	util::WeakHandle<TComponent> pragma::BaseEntityComponentSystem::AddComponent(bool bForceCreateNew)
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
		return {};
	auto whComponent = AddComponent(componentId,bForceCreateNew);
	if(whComponent.expired())
		return {};
	return util::WeakHandle<TComponent>(std::static_pointer_cast<TComponent>(whComponent.get()->shared_from_this()));
}
template<class TComponent,typename>
	void pragma::BaseEntityComponentSystem::RemoveComponent()
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
		return;
	RemoveComponent(componentId);
}
template<class TComponent,typename>
	util::WeakHandle<TComponent> pragma::BaseEntityComponentSystem::GetComponent() const
{
	ComponentId componentId;
	if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)),componentId) == false)
		return util::WeakHandle<TComponent>{};
	auto it = m_componentLookupTable.find(componentId);
	return (it != m_componentLookupTable.end()) ? util::WeakHandle<TComponent>(std::static_pointer_cast<TComponent>(it->second.lock())) : util::WeakHandle<TComponent>{};
}
template<class TComponent,typename>
	bool pragma::BaseEntityComponentSystem::HasComponent() const
{
	ComponentId componentId;
	if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)),componentId) == false)
		return false;
	auto it = m_componentLookupTable.find(componentId);
	return it != m_componentLookupTable.end() && it->second.expired() == false;
}

#endif
