// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"
#include <cassert>

export module pragma.shared:entities.manager;

export import :util.global_string_table;

export import :entities.enums;
export import :entities.components.events.event_info;
export import :entities.member_info;
export import :entities.member_type;
export import pragma.lua;

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma {
		DLLNETWORK std::string get_normalized_component_member_name(const std::string &name);
		DLLNETWORK size_t get_component_member_name_hash(const std::string &name);
		DLLNETWORK size_t get_component_member_name_hash(const char *name);

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
			GString categoryPath = "";
			Flags flags = Flags::None;

			ComponentRegInfo(GString category = "", Flags flags = Flags::None);
			ComponentRegInfo(Flags flags);
		};

		struct DLLNETWORK ComponentInfo {
			ComponentInfo() = default;
			ComponentInfo(const ComponentInfo &other);
			ComponentInfo(ComponentInfo &&other);
			ComponentInfo &operator=(const ComponentInfo &other);
			ComponentInfo &operator=(ComponentInfo &&other);
			GString name = "";
			GString category = "";
			std::function<util::TSharedHandle<BaseEntityComponent>(ecs::BaseEntity &)> factory = nullptr;
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

			util::TSharedHandle<BaseEntityComponent> CreateComponent(const std::string &name, ecs::BaseEntity &ent) const;
			util::TSharedHandle<BaseEntityComponent> CreateComponent(ComponentId componentId, ecs::BaseEntity &ent) const;
			ComponentId PreRegisterComponentType(const std::string &name);
			ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(ecs::BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags, std::type_index typeIndex);
			ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(ecs::BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags);
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

			void RegisterLuaBindings(lua::State *l, luabind::module_ &module);

			// Automatically called when a component was removed; Don't call this manually!
			void DeregisterComponent(BaseEntityComponent &component);
		  private:
			ComponentId RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(ecs::BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags, const std::type_index *typeIndex);
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
			std::vector<void (*)(lua::State *, luabind::module_ &)> m_luaBindingRegistrations;
			ComponentId m_nextComponentId = 0u;

			// List of all created components by component id
			mutable std::vector<ComponentContainerInfo> m_components;

			std::unordered_map<ComponentEventId, ComponentEventInfo> m_componentEvents;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::ComponentFlags)

	namespace pragma {
		template<class TComponent, typename>
		ComponentId EntityComponentManager::RegisterComponentType(const std::string &name, const ComponentRegInfo &regInfo)
		{
			auto flags = ComponentFlags::None;
			if(std::is_base_of<BaseNetComponent, TComponent>::value)
				flags |= ComponentFlags::Networked;
			auto componentId = PreRegisterComponentType(name);
			TComponent::RegisterEvents(*this, [this, componentId](const std::string &evName, ComponentEventInfo::Type type) {
				auto id = RegisterEvent<TComponent>(evName, type);
				auto it = m_componentEvents.find(id);
				assert(it != m_componentEvents.end());
				it->second.componentId = componentId;
				return id;
			});
			RegisterComponentType(name, [](ecs::BaseEntity &ent) { return pragma::util::TSharedHandle<BaseEntityComponent> {new TComponent {ent}, [](BaseEntityComponent *c) { delete c; }}; }, regInfo, flags, std::type_index(typeid(TComponent)));
			auto &componentInfo = *m_componentInfos[componentId];
			TComponent::RegisterMembers(*this, [this, &componentInfo](ComponentMemberInfo &&memberInfo) -> ComponentMemberIndex { return RegisterMember(componentInfo, std::move(memberInfo)); });
			if(m_luaBindingRegistrations.size() == m_luaBindingRegistrations.capacity())
				m_luaBindingRegistrations.reserve(m_luaBindingRegistrations.size() * 1.5 + 50);
			m_luaBindingRegistrations.push_back(&TComponent::RegisterLuaBindings);
			return componentId;
		}

		template<class TComponent, typename>
		bool EntityComponentManager::GetComponentTypeId(ComponentId &outId) const
		{
			auto it = m_typeIndexToComponentId.find(std::type_index(typeid(TComponent)));
			if(it == m_typeIndexToComponentId.end())
				return false;
			outId = it->second;
			return true;
		}
	}
};

export namespace pragma {
	template<typename TComponent, typename T, auto TSetter, auto TGetter, typename TSpecializationType>
	    requires(is_valid_component_property_type_v<T> && (std::is_same_v<TSpecializationType, AttributeSpecializationType> || util::is_string<TSpecializationType>::value))
	ComponentMemberInfo create_component_member_info(std::string &&name, std::optional<T> defaultValue, TSpecializationType specialization)
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
	ComponentMemberInfo create_component_member_info(std::string &&name, std::optional<T> defaultValue = {})
	{
		return create_component_member_info<TComponent, T, TSetter, TGetter, AttributeSpecializationType>(std::move(name), std::move(defaultValue), AttributeSpecializationType::None);
	}
};
