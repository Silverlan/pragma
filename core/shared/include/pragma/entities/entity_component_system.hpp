/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

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

		// For internal use only
		EntityComponentManager *GetComponentManager();
		const EntityComponentManager *GetComponentManager() const;
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

#endif
