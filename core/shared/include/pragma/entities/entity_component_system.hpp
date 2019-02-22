#ifndef __ENTITY_COMPONENT_SYSTEM_HPP__
#define __ENTITY_COMPONENT_SYSTEM_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/util/util_handled.hpp"
#include <sharedutils/util_weak_handle.hpp>
#include <memory>
#include <vector>
#include <typeindex>

class BaseEntity;
namespace pragma
{
	using ComponentId = uint32_t;
	using ComponentEventId = uint32_t;
	class EntityComponentManager;
	class BaseEntityComponent;
	struct ComponentEvent;
	class DLLNETWORK BaseEntityComponentSystem
	{
	public:
		virtual ~BaseEntityComponentSystem();
		util::EventReply BroadcastEvent(ComponentEventId ev,ComponentEvent &evData,const BaseEntityComponent *src=nullptr) const;
		util::EventReply BroadcastEvent(ComponentEventId ev) const;

		util::WeakHandle<pragma::BaseEntityComponent> AddComponent(const std::string &name,bool bForceCreateNew=false);
		util::WeakHandle<pragma::BaseEntityComponent> AddComponent(ComponentId componentId,bool bForceCreateNew=false);
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			util::WeakHandle<TComponent> AddComponent(bool bForceCreateNew=false);
		// This will remove ALL components of this type
		void RemoveComponent(ComponentId componentId);
		void RemoveComponent(const std::string &name);

		void RemoveComponent(pragma::BaseEntityComponent &component);
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			void RemoveComponent();

		// Removes ALL components
		void ClearComponents();

		// Note: shared_ptr should never be stored outside of this object type
		const std::vector<std::shared_ptr<BaseEntityComponent>> &GetComponents() const;
		std::vector<std::shared_ptr<BaseEntityComponent>> &GetComponents();

		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			util::WeakHandle<TComponent> GetComponent() const;

		bool HasComponent(ComponentId componentId) const;
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			bool HasComponent() const;

		util::WeakHandle<BaseEntityComponent> FindComponent(ComponentId componentId) const;
		util::WeakHandle<BaseEntityComponent> FindComponent(const std::string &name) const;
	protected:
		BaseEntityComponentSystem()=default;

		void Initialize(BaseEntity &ent,EntityComponentManager &componentManager);
		virtual void OnComponentAdded(BaseEntityComponent &component);
		virtual void OnComponentRemoved(BaseEntityComponent &component);
	private:
		std::unordered_map<ComponentId,std::weak_ptr<BaseEntityComponent>> m_componentLookupTable; // Only contains one (the first) component per type; Used for fast lookups
		std::vector<std::shared_ptr<BaseEntityComponent>> m_components;
		EntityComponentManager *m_componentManager;
		BaseEntity *m_entity;
	};
};

template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
	util::WeakHandle<TComponent> pragma::BaseEntityComponentSystem::AddComponent(bool bForceCreateNew)
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
		return nullptr;
	auto whComponent = AddComponent(componentId,bForceCreateNew);
	if(whComponent.expired())
		return {};
	return util::WeakHandle<TComponent>(std::static_pointer_cast<TComponent>(whComponent.get()->shared_from_this()));
}
template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
	void pragma::BaseEntityComponentSystem::RemoveComponent()
{
	ComponentId componentId;
	if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
		return;
	RemoveComponent(componentId);
}
template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
	util::WeakHandle<TComponent> pragma::BaseEntityComponentSystem::GetComponent() const
{
	ComponentId componentId;
	if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)),componentId) == false)
		return util::WeakHandle<TComponent>{};
	auto it = m_componentLookupTable.find(componentId);
	return (it != m_componentLookupTable.end()) ? util::WeakHandle<TComponent>(std::static_pointer_cast<TComponent>(it->second.lock())) : util::WeakHandle<TComponent>{};
}
template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
	bool pragma::BaseEntityComponentSystem::HasComponent() const
{
	ComponentId componentId;
	if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)),componentId) == false)
		return false;
	auto it = m_componentLookupTable.find(componentId);
	return it != m_componentLookupTable.end() && it->second.expired() == false;
}

#endif
