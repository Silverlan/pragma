/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_COMPONENT_MANAGER_HPP__
#define __ENTITY_COMPONENT_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/entity_component_info.hpp"
#include "pragma/entities/component_member_reference.hpp"
#include "pragma/entities/member_type.hpp"
#include "pragma/entities/entity_component_info.hpp"
#include "pragma/entities/entity_component_member_info.hpp"
#include "pragma/entities/entity_component_event_info.hpp"
#include "pragma/util/global_string_table.hpp"
#include "pragma/types.hpp"
#include <cinttypes>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <queue>
#include <mathutil/umath.h>
#include <sharedutils/functioncallback.h>
#ifdef __linux__
#include "pragma/entities/components/base_entity_component.hpp"
#endif

namespace udm {
	enum class Type : uint8_t;
	struct Element;
	struct Property;
	using PProperty = std::shared_ptr<Property>;
};

class BaseEntity;
namespace pragma {
	class BaseEntityComponent;
	DLLNETWORK std::string get_normalized_component_member_name(const std::string &name);
	DLLNETWORK size_t get_component_member_name_hash(const std::string &name);
	DLLNETWORK size_t get_component_member_name_hash(const char *name);

	enum class AttributeSpecializationType : uint8_t;
	enum class ComponentMemberFlags : uint32_t;
	enum class ComponentFlags : uint8_t {
		None = 0u,
		Networked = 1u,

		// Component isn't networked, but wants to be.
		// (e.g. because a networked event has been registered).
		// In this case the component will be networked the next time
		// it is created. Note: This flag only works for
		// Lua-based components! It also has no effect if the
		// component has already been created at least one
		// in the past.
		MakeNetworked = Networked << 1u,

		LuaBased = MakeNetworked << 1u
	};
	class DLLNETWORK BaseNetComponent {
	  public:
		virtual bool ShouldTransmitNetData() const = 0;
	  protected:
		BaseNetComponent() = default;
	};

	struct DLLNETWORK ComponentInfo {
		ComponentInfo() = default;
		ComponentInfo(const ComponentInfo &other);
		ComponentInfo(ComponentInfo &&other);
		ComponentInfo &operator=(const ComponentInfo &other);
		ComponentInfo &operator=(ComponentInfo &&other);
		pragma::GString name = "";
		std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity &)> factory = nullptr;
		mutable std::unique_ptr<std::vector<CallbackHandle>> onCreateCallbacks = nullptr;
		ComponentId id = std::numeric_limits<uint32_t>::max();
		ComponentFlags flags = ComponentFlags::None;
		std::vector<ComponentMemberInfo> members;
		std::unordered_map<std::string, ComponentMemberIndex> memberNameToIndex;
		std::optional<ComponentMemberIndex> FindMember(const std::string &name) const;

		bool IsValid() const { return factory != nullptr; }
	};

	class DLLNETWORK EntityComponentManager {
	  public:
		EntityComponentManager() = default;
		EntityComponentManager(const EntityComponentManager &) = delete;
		EntityComponentManager &operator=(const EntityComponentManager &) = delete;

		util::TSharedHandle<BaseEntityComponent> CreateComponent(const std::string &name, BaseEntity &ent) const;
		util::TSharedHandle<BaseEntityComponent> CreateComponent(ComponentId componentId, BaseEntity &ent) const;
		ComponentId PreRegisterComponentType(const std::string &name);
		ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity &)> &factory, ComponentFlags flags, std::type_index typeIndex);
		ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity &)> &factory, ComponentFlags flags);
		template<class TComponent, typename = std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent, TComponent>::value>>
		ComponentId RegisterComponentType(const std::string &name);
		bool GetComponentTypeId(const std::string &name, ComponentId &outId, bool bIncludePreregistered = true) const;
		template<class TComponent, typename = std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent, TComponent>::value>>
		bool GetComponentTypeId(ComponentId &outId) const;
		bool GetComponentTypeIndex(ComponentId componentId, std::type_index &typeIndex) const;
		bool GetComponentId(std::type_index typeIndex, ComponentId &componentId) const;
		const std::vector<std::unique_ptr<ComponentInfo>> &GetRegisteredComponentTypes() const;
		ComponentMemberIndex RegisterMember(ComponentInfo &componentInfo, ComponentMemberInfo &&memberInfo);

		void LinkComponentType(ComponentId linkFrom, ComponentId linkTo);
		CallbackHandle AddCreationCallback(ComponentId componentId, const std::function<void(std::reference_wrapper<BaseEntityComponent>)> &onCreate);
		CallbackHandle AddCreationCallback(const std::string &componentName, const std::function<void(std::reference_wrapper<BaseEntityComponent>)> &onCreate);

		const ComponentInfo *GetComponentInfo(ComponentId id) const;
		ComponentInfo *GetComponentInfo(ComponentId id);

		template<class T>
		ComponentEventId RegisterEvent(const std::string &evName, ComponentEventInfo::Type type = ComponentEventInfo::Type::Broadcast)
		{
			return RegisterEvent(evName, typeid(T), type);
		}
		ComponentEventId RegisterEvent(const std::string &evName, std::type_index typeIndex, ComponentEventInfo::Type type = ComponentEventInfo::Type::Broadcast);
		ComponentEventId RegisterEventById(const std::string &evName, ComponentId componentId, ComponentEventInfo::Type type = ComponentEventInfo::Type::Broadcast);
		std::optional<ComponentEventId> FindEventId(ComponentId componentId, const std::string &evName) const;
		std::optional<ComponentEventId> FindEventId(const std::string &componentName, const std::string &evName) const;
		bool GetEventId(const std::string &evName, ComponentEventId &evId) const;
		ComponentEventId GetEventId(const std::string &evName) const;
		bool GetEventName(ComponentEventId evId, std::string &outEvName) const;
		std::string GetEventName(ComponentEventId evId) const;
		const std::unordered_map<ComponentEventId, ComponentEventInfo> &GetEvents() const;

		struct DLLNETWORK ComponentContainerInfo {
		  protected:
			friend EntityComponentManager;
			void Push(BaseEntityComponent &component);
			void Pop(BaseEntityComponent &component);
			std::size_t GetCount() const;
			const std::vector<BaseEntityComponent *> &GetComponents() const;
		  private:
			std::queue<std::size_t> m_freeIndices = {};
			std::size_t m_count = 0ull;
			std::vector<BaseEntityComponent *> m_components = {};
		};

		const std::vector<ComponentContainerInfo> &GetComponents() const;
		std::vector<ComponentContainerInfo> &GetComponents();
		// Returns all currently active components of the specified type. Note that some items in the container may be NULL.
		const std::vector<BaseEntityComponent *> &GetComponents(ComponentId componentId) const;
		const std::vector<BaseEntityComponent *> &GetComponents(ComponentId componentId, std::size_t &count) const;

		// Automatically called when a component was removed; Don't call this manually!
		void DeregisterComponent(BaseEntityComponent &component);
	  private:
		ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity &)> &factory, ComponentFlags flags, const std::type_index *typeIndex);
		virtual void OnComponentTypeRegistered(const ComponentInfo &componentInfo);

		struct ComponentTypeLinkInfo {
			ComponentId targetType;
			CallbackHandle onCreateCallback;
		};
		std::vector<std::unique_ptr<ComponentInfo>> m_preRegistered;
		std::vector<std::unique_ptr<ComponentInfo>> m_componentInfos;
		std::unordered_map<std::type_index, ComponentId> m_typeIndexToComponentId;
		std::unordered_map<ComponentId, std::vector<ComponentTypeLinkInfo>> m_linkedComponentTypes;
		std::vector<std::shared_ptr<std::type_index>> m_componentIdToTypeIndex;
		ComponentId m_nextComponentId = 0u;

		// List of all created components by component id
		mutable std::vector<ComponentContainerInfo> m_components;

		std::unordered_map<ComponentEventId, ComponentEventInfo> m_componentEvents;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ComponentFlags);

template<class TComponent, typename>
pragma::ComponentId pragma::EntityComponentManager::RegisterComponentType(const std::string &name)
{
	auto flags = ComponentFlags::None;
	if(std::is_base_of<pragma::BaseNetComponent, TComponent>::value)
		flags |= ComponentFlags::Networked;
	auto componentId = PreRegisterComponentType(name);
	TComponent::RegisterEvents(*this, [this, componentId](const std::string &evName, ComponentEventInfo::Type type) {
		auto id = RegisterEvent<TComponent>(evName, type);
		auto it = m_componentEvents.find(id);
		assert(it != m_componentEvents.end());
		it->second.componentId = componentId;
		return id;
	});
	RegisterComponentType(
	  name,
	  [](BaseEntity &ent) {
		  return util::TSharedHandle<BaseEntityComponent> {new TComponent {ent}, [](pragma::BaseEntityComponent *c) { delete c; }};
	  },
	  flags, std::type_index(typeid(TComponent)));
	auto &componentInfo = *m_componentInfos[componentId];
	TComponent::RegisterMembers(*this, [this, &componentInfo](ComponentMemberInfo &&memberInfo) -> ComponentMemberIndex { return RegisterMember(componentInfo, std::move(memberInfo)); });
	return componentId;
}

template<class TComponent, typename>
bool pragma::EntityComponentManager::GetComponentTypeId(ComponentId &outId) const
{
	auto it = m_typeIndexToComponentId.find(std::type_index(typeid(TComponent)));
	if(it == m_typeIndexToComponentId.end())
		return false;
	outId = it->second;
	return true;
}

#endif
