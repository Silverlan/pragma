// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"
#include <cinttypes>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <queue>
#include <mathutil/umath.h>
#include "sharedutils/magic_enum.hpp"
#include "sharedutils/util_shared_handle.hpp"
#include <sharedutils/functioncallback.h>
#include "udm.hpp"

export module pragma.shared:entities.manager;

export import :util.global_string_table;

export import :entities.enums;
export import :entities.components.events.event_info;
export import :entities.member_info;
export import :entities.member_type;

export {
	class BaseEntity;
	namespace pragma {
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

			LuaBased = MakeNetworked << 1u,
			HideInEditor = LuaBased << 1u,
		};
		class DLLNETWORK BaseNetComponent {
		public:
			virtual bool ShouldTransmitNetData() const = 0;
		protected:
			BaseNetComponent() = default;
		};

		struct DLLNETWORK ComponentRegInfo {
			enum class Flags : uint8_t {
				None = 0,
				HideInEditor = 1,
			};
			pragma::GString categoryPath = "";
			Flags flags = Flags::None;

			ComponentRegInfo(pragma::GString category = "", Flags flags = Flags::None);
			ComponentRegInfo(Flags flags);
		};

		struct DLLNETWORK ComponentInfo {
			ComponentInfo() = default;
			ComponentInfo(const ComponentInfo &other);
			ComponentInfo(ComponentInfo &&other);
			ComponentInfo &operator=(const ComponentInfo &other);
			ComponentInfo &operator=(ComponentInfo &&other);
			pragma::GString name = "";
			pragma::GString category = "";
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
			ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags, std::type_index typeIndex);
			ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags);
			template<class TComponent, typename = std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent, TComponent>::value>>
			ComponentId RegisterComponentType(const std::string &name, const ComponentRegInfo &regInfo);
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

			void RegisterLuaBindings(lua_State *l, luabind::module_ &module);

			// Automatically called when a component was removed; Don't call this manually!
			void DeregisterComponent(BaseEntityComponent &component);
		private:
			ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags, const std::type_index *typeIndex);
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
			std::vector<void (*)(lua_State *, luabind::module_ &)> m_luaBindingRegistrations;
			ComponentId m_nextComponentId = 0u;

			// List of all created components by component id
			mutable std::vector<ComponentContainerInfo> m_components;

			std::unordered_map<ComponentEventId, ComponentEventInfo> m_componentEvents;
		};
	};
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::ComponentFlags);

	template<class TComponent, typename>
	pragma::ComponentId pragma::EntityComponentManager::RegisterComponentType(const std::string &name, const ComponentRegInfo &regInfo)
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
		RegisterComponentType(name, [](BaseEntity &ent) { return util::TSharedHandle<BaseEntityComponent> {new TComponent {ent}, [](pragma::BaseEntityComponent *c) { delete c; }}; }, regInfo, flags, std::type_index(typeid(TComponent)));
		auto &componentInfo = *m_componentInfos[componentId];
		TComponent::RegisterMembers(*this, [this, &componentInfo](ComponentMemberInfo &&memberInfo) -> ComponentMemberIndex { return RegisterMember(componentInfo, std::move(memberInfo)); });
		if(m_luaBindingRegistrations.size() == m_luaBindingRegistrations.capacity())
			m_luaBindingRegistrations.reserve(m_luaBindingRegistrations.size() * 1.5 + 50);
		m_luaBindingRegistrations.push_back(&TComponent::RegisterLuaBindings);
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
};

export namespace pragma {
	template<typename TComponent, typename T, auto TSetter, auto TGetter, typename TSpecializationType>
	    requires(is_valid_component_property_type_v<T> && (std::is_same_v<TSpecializationType, AttributeSpecializationType> || util::is_string<TSpecializationType>::value))
	static ComponentMemberInfo create_component_member_info(std::string &&name, std::optional<T> defaultValue, TSpecializationType specialization)
	{
		auto memberInfo = ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(std::move(name));
		memberInfo.type = ents::member_type_to_enum<T>();
		memberInfo.SetGetterFunction<TComponent, T, TGetter>();
		memberInfo.SetSetterFunction<TComponent, T, TSetter>();
		memberInfo.SetSpecializationType(specialization);
		if constexpr(std::is_enum_v<T>) {
			memberInfo.SetEnum(
			  [](const std::string &name) -> int64_t {
				  auto v = magic_enum::enum_cast<T>(name);
				  if(!v.has_value())
					  return {};
				  return static_cast<int64_t>(*v);
			  },
			  [](int64_t value) -> std::optional<std::string> {
				  auto e = magic_enum::enum_name<T>(static_cast<T>(value));
				  if(e.empty())
					  return {};
				  return std::string {e};
			  },
			  []() -> std::vector<int64_t> {
				  auto &enumValues = magic_enum::enum_values<T>();
				  std::vector<int64_t> rvalues;
				  rvalues.resize(enumValues.size());
				  for(auto i = decltype(enumValues.size()) {0u}; i < enumValues.size(); ++i)
					  rvalues[i] = static_cast<int64_t>(enumValues[i]);
				  return rvalues;
			  });
			auto &enumValues = magic_enum::enum_values<T>();
			if(!enumValues.empty()) {
				memberInfo.SetMin(static_cast<float>(enumValues.front()));
				memberInfo.SetMax(static_cast<float>(enumValues.back()));
			}
		}
		if(defaultValue.has_value())
			memberInfo.SetDefault<T>(*defaultValue);
		return memberInfo;
	}

	template<typename TComponent, typename T, auto TSetter, auto TGetter>
	    requires(is_valid_component_property_type_v<T>)
	static ComponentMemberInfo create_component_member_info(std::string &&name, std::optional<T> defaultValue = {})
	{
		return create_component_member_info<TComponent, T, TSetter, TGetter, AttributeSpecializationType>(std::move(name), std::move(defaultValue), AttributeSpecializationType::None);
	}

	template<typename T>
	void ComponentMemberInfo::SetDefault(T value)
	{
		if(ents::member_type_to_enum<T>() != type)
			throw std::runtime_error {"Unable to set default member value: Value type " + std::string {magic_enum::enum_name(ents::member_type_to_enum<T>())} + " does not match member type " + std::string {magic_enum::enum_name(type)} + "!"};
		m_default = std::unique_ptr<void, void (*)(void *)> {new T {std::move(value)}, [](void *ptr) { delete static_cast<T *>(ptr); }};
	}

	template<typename T>
	bool ComponentMemberInfo::GetDefault(T &outValue) const
	{
		if(!m_default || ents::member_type_to_enum<T>() != type)
			return false;
		outValue = *static_cast<T *>(m_default.get());
		return true;
	}
};
