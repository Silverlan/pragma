// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.system;

export import :entities.components.base;
export import :entities.components.events.event;
export import :entities.components.handle;
export import :entities.manager;
export import :entities.enums;

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma {
		class EntityComponentManager;
		class DLLNETWORK BaseEntityComponentSystem {
		  public:
			enum class StateFlags : uint32_t {
				None = 0,
				// A component was removed and it's entry set to NULL
				ComponentCleanupRequired = 1u,
				IsBeingRemoved = ComponentCleanupRequired << 1u
			};
			virtual ~BaseEntityComponentSystem();
			util::EventReply BroadcastEvent(ComponentEventId ev, ComponentEvent &evData, const BaseEntityComponent *src = nullptr) const;
			util::EventReply BroadcastEvent(ComponentEventId ev) const;

			ComponentHandle<BaseEntityComponent> AddComponent(const std::string &name, bool bForceCreateNew = false);
			ComponentHandle<BaseEntityComponent> AddComponent(ComponentId componentId, bool bForceCreateNew = false);
			template<class TComponent, typename = std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent, TComponent>::value>>
			ComponentHandle<TComponent> AddComponent(bool bForceCreateNew = false);
			// This will remove ALL components of this type
			void RemoveComponent(ComponentId componentId);
			void RemoveComponent(const std::string &name);

			void RemoveComponent(BaseEntityComponent &component);
			template<class TComponent, typename = std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent, TComponent>::value>>
			void RemoveComponent();

			// Removes ALL components
			void ClearComponents();

			// Note: TSharedHandle should never be stored outside of this object type
			const std::vector<util::TSharedHandle<BaseEntityComponent>> &GetComponents() const;
			std::vector<util::TSharedHandle<BaseEntityComponent>> &GetComponents();

			template<class TComponent, typename = std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent, TComponent>::value>>
			ComponentHandle<TComponent> GetComponent() const;

			bool HasComponent(ComponentId componentId) const;
			template<class TComponent, typename = std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent, TComponent>::value>>
			bool HasComponent() const;

			ComponentHandle<BaseEntityComponent> FindComponent(ComponentId componentId) const;
			ComponentHandle<BaseEntityComponent> FindComponent(const std::string &name) const;

			virtual void OnRemove() = 0;

			// For internal use only
			EntityComponentManager *GetComponentManager();
			const EntityComponentManager *GetComponentManager() const;
			static void Cleanup();
		  protected:
			BaseEntityComponentSystem() = default;

			void Initialize(ecs::BaseEntity &ent, EntityComponentManager &componentManager);
			virtual void OnComponentAdded(BaseEntityComponent &component);
			virtual void OnComponentRemoved(BaseEntityComponent &component);
		  private:
			std::unordered_map<ComponentId, ComponentHandle<BaseEntityComponent>> m_componentLookupTable; // Only contains one (the first) component per type; Used for fast lookups
			std::vector<util::TSharedHandle<BaseEntityComponent>> m_components;
			EntityComponentManager *m_componentManager;
			ecs::BaseEntity *m_entity;
			mutable StateFlags m_stateFlags = StateFlags::None;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseEntityComponentSystem::StateFlags)

	namespace pragma {
		template<class TComponent, typename>
		ComponentHandle<TComponent> BaseEntityComponentSystem::AddComponent(bool bForceCreateNew)
		{
			ComponentId componentId;
			if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
				return {};
			auto whComponent = AddComponent(componentId, bForceCreateNew);
			if(whComponent.expired())
				return {};
			return whComponent->template GetHandle<TComponent>();
		}
		template<class TComponent, typename>
		void BaseEntityComponentSystem::RemoveComponent()
		{
			ComponentId componentId;
			if(m_componentManager->GetComponentTypeId<TComponent>(componentId) == false)
				return;
			RemoveComponent(componentId);
		}
		template<class TComponent, typename>
		ComponentHandle<TComponent> BaseEntityComponentSystem::GetComponent() const
		{
			ComponentId componentId;
			if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)), componentId) == false)
				return ComponentHandle<TComponent> {};
			auto it = m_componentLookupTable.find(componentId);
			return (it != m_componentLookupTable.end()) ? const_cast<BaseEntityComponent *>(it->second.get())->GetHandle<TComponent>() : ComponentHandle<TComponent> {};
		}
		template<class TComponent, typename>
		bool BaseEntityComponentSystem::HasComponent() const
		{
			ComponentId componentId;
			if(m_componentManager->GetComponentId(std::type_index(typeid(TComponent)), componentId) == false)
				return false;
			auto it = m_componentLookupTable.find(componentId);
			return it != m_componentLookupTable.end() && it->second.expired() == false;
		}
	}
};
