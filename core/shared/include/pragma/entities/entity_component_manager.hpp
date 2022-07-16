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

namespace udm
{
	enum class Type : uint8_t;
	struct Element;
	struct Property;
	using PProperty = std::shared_ptr<Property>;
};

class BaseEntity;
namespace pragma
{
	class BaseEntityComponent;
	DLLNETWORK std::string get_normalized_component_member_name(const std::string &name);
	DLLNETWORK size_t get_component_member_name_hash(const std::string &name);

	enum class AttributeSpecializationType : uint8_t;
	enum class ComponentMemberFlags : uint32_t;
	struct DLLNETWORK ComponentMemberInfo
	{
		struct DLLNETWORK EnumConverter
		{
			using NameToEnumFunction = std::function<std::optional<int64_t>(const std::string&)>;
			using EnumToNameFunction = std::function<std::optional<std::string>(int64_t)>;
			using EnumValueGetFunction = std::function<std::vector<int64_t>()>;
			NameToEnumFunction nameToEnum;
			EnumToNameFunction enumToName;
			EnumValueGetFunction getValues;
		};
		using ApplyFunction = void(*)(const ComponentMemberInfo&,BaseEntityComponent&,const void*);
		using GetFunction = void(*)(const ComponentMemberInfo&,BaseEntityComponent&,void*);
		using InterpolationFunction = void(*)(const void*,const void*,double,void*);
		using UpdateDependenciesFunction = void(*)(BaseEntityComponent&,std::vector<std::string>&);
		static ComponentMemberInfo CreateDummy();
		ComponentMemberInfo(std::string &&name,ents::EntityMemberType type,const ApplyFunction &applyFunc,const GetFunction &getFunc);

		template<typename TComponent,typename T,void(*TApply)(const ComponentMemberInfo&,TComponent&,const T&)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,const void *value) {
				TApply(memberInfo,static_cast<TComponent&>(component),*static_cast<const T*>(value));
			};
		}
		template<typename TComponent,typename T,void(*TApply)(const ComponentMemberInfo&,TComponent&,T)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,const void *value) {
				TApply(memberInfo,static_cast<TComponent&>(component),*static_cast<const T*>(value));
			};
		}
		template<typename TComponent,typename T,void(TComponent::*TApply)(const T&)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,const void *value) {
				(static_cast<TComponent&>(component).*TApply)(*static_cast<const T*>(value));
			};
		}
		template<typename TComponent,typename T,void(TComponent::*TApply)(T)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,const void *value) {
				(static_cast<TComponent&>(component).*TApply)(*static_cast<const T*>(value));
			};
		}

		template<typename TComponent,typename T,void(*TGetter)(const ComponentMemberInfo&,TComponent&,T&)>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,void *value) {
				TGetter(memberInfo,static_cast<TComponent&>(component),*static_cast<T*>(value));
			};
		}
		template<typename TComponent,typename T,auto(*TGetter)()>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,void *value) {
				*static_cast<T*>(value) = TGetter();
			};
		}
		template<typename TComponent,typename T,auto(TComponent::*TGetter)()>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,void *value) {
				*static_cast<T*>(value) = (static_cast<TComponent&>(component).*TGetter)();
			};
		}
		template<typename TComponent,typename T,auto(TComponent::*TGetter)() const>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo,BaseEntityComponent &component,void *value) {
				*static_cast<T*>(value) = (static_cast<TComponent&>(component).*TGetter)();
			};
		}

		template<typename TComponent,typename T,void(*TInterp)(const T&,const T&,double,T&)>
		void SetInterpolationFunction()
		{
			interpolationFunction = [](const void *v0,const void *v1,double t,void *out) {
				TInterp(*static_cast<const T*>(v0),*static_cast<const T*>(v1),t,*static_cast<T*>(out));
			};
		}

		void SetName(const std::string &name);
		void SetName(std::string &&name);
		const std::string &GetName() const {return m_name;}
		size_t GetNameHash() const {return m_nameHash;}

		AttributeSpecializationType GetSpecializationType() const {return m_specializationType;}
		const std::string *GetCustomSpecializationType() const {return m_customSpecializationType.get();}
		void SetSpecializationType(AttributeSpecializationType type);
		void SetSpecializationType(std::string customType);
		
		ComponentMemberInfo(const ComponentMemberInfo&);
		ComponentMemberInfo &operator=(const ComponentMemberInfo &other);

		void SetMin(float min);
		void SetMax(float max);
		void SetStepSize(float stepSize);
		udm::Property &AddMetaData();
		void AddMetaData(const udm::PProperty &prop);
		const udm::PProperty &GetMetaData() const;
		std::optional<float> GetMin() const {return m_min;}
		std::optional<float> GetMax() const {return m_max;}
		std::optional<float> GetStepSize() const {return m_stepSize;}
		template<typename T>
			bool GetDefault(T &outValue) const;
		template<typename T>
			void SetDefault(T value);

		void SetEnum(
			const EnumConverter::NameToEnumFunction &nameToEnum,
			const EnumConverter::EnumToNameFunction &enumToName,
			const EnumConverter::EnumValueGetFunction &getValues
		);
		bool IsEnum() const;
		std::optional<int64_t> EnumNameToValue(const std::string &name) const;
		std::optional<std::string> ValueToEnumName(int64_t value) const;
		bool GetEnumValues(std::vector<int64_t> &outValues) const;

		void UpdateDependencies(BaseEntityComponent &component,std::vector<std::string> &outAffectedProps);
		void ResetToDefault(BaseEntityComponent &component);

		void SetFlags(ComponentMemberFlags flags);
		ComponentMemberFlags GetFlags() const;
		bool HasFlag(ComponentMemberFlags flag) const;
		void SetFlag(ComponentMemberFlags flag,bool set=true);

		ents::EntityMemberType type;
		ApplyFunction setterFunction = nullptr;
		GetFunction getterFunction = nullptr;
		InterpolationFunction interpolationFunction = nullptr;
		UpdateDependenciesFunction updateDependenciesFunction = nullptr;

		union
		{
			uint64_t userIndex;
			void *userData = nullptr;
		};
	private:
		ComponentMemberInfo();
		std::string m_name;
		size_t m_nameHash = 0;
		ComponentMemberFlags m_flags = static_cast<ComponentMemberFlags>(0);

		AttributeSpecializationType m_specializationType;
		std::unique_ptr<std::string> m_customSpecializationType = nullptr;
		
		udm::PProperty m_metaData = nullptr;
		std::optional<float> m_min {};
		std::optional<float> m_max {};
		std::optional<float> m_stepSize {};
		std::unique_ptr<void,void(*)(void*)> m_default = std::unique_ptr<void,void(*)(void*)>{nullptr,[](void*) {}};
		std::unique_ptr<EnumConverter> m_enumConverter = nullptr;
	};

	enum class ComponentFlags : uint8_t
	{
		None = 0u,
		Networked = 1u,

		// Component isn't networked, but wants to be.
		// (e.g. because a networked event has been registered).
		// In this case the component will be networked the next time
		// it is created. Note: This flag only works for
		// Lua-based components! It also has no effect if the
		// component has already been created at least one
		// in the past.
		MakeNetworked = Networked<<1u,

		LuaBased = MakeNetworked<<1u
	};
	class DLLNETWORK BaseNetComponent
	{
	public:
		virtual bool ShouldTransmitNetData() const=0;
	protected:
		BaseNetComponent()=default;
	};

	struct DLLNETWORK ComponentInfo
	{
		ComponentInfo()=default;
		ComponentInfo(const ComponentInfo &other);
		ComponentInfo(ComponentInfo &&other);
		ComponentInfo &operator=(const ComponentInfo &other);
		ComponentInfo &operator=(ComponentInfo &&other);
		std::string name;
		std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> factory = nullptr;
		mutable std::unique_ptr<std::vector<CallbackHandle>> onCreateCallbacks = nullptr;
		ComponentId id = std::numeric_limits<uint32_t>::max();
		ComponentFlags flags = ComponentFlags::None;
		std::vector<ComponentMemberInfo> members;
		std::unordered_map<std::string,ComponentMemberIndex> memberNameToIndex;
		std::optional<ComponentMemberIndex> FindMember(const std::string &name) const;

		bool IsValid() const {return factory != nullptr;}
	};

	class DLLNETWORK EntityComponentManager
	{
	public:
		EntityComponentManager()=default;
		EntityComponentManager(const EntityComponentManager&)=delete;
		EntityComponentManager &operator=(const EntityComponentManager&)=delete;

		struct DLLNETWORK EventInfo
		{
			enum class Type : uint8_t
			{
				Broadcast = 0,
				Explicit
			};
			EventInfo(const std::string &name,std::optional<ComponentId> componentId={},std::optional<std::type_index> typeIndex={},Type type=Type::Broadcast)
				: name(name),type{type},typeIndex{typeIndex},componentId{componentId.has_value() ? *componentId : INVALID_COMPONENT_ID}
			{}
			ComponentEventId id = std::numeric_limits<ComponentEventId>::max();
			std::string name;
			ComponentId componentId = INVALID_COMPONENT_ID;
			Type type = Type::Broadcast;

			// Only set if this is a C++ component
			std::optional<std::type_index> typeIndex {};
		};

		util::TSharedHandle<BaseEntityComponent> CreateComponent(const std::string &name,BaseEntity &ent) const;
		util::TSharedHandle<BaseEntityComponent> CreateComponent(ComponentId componentId,BaseEntity &ent) const;
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			util::TSharedHandle<TComponent> CreateComponent(BaseEntity &ent) const;
		ComponentId PreRegisterComponentType(const std::string &name);
		ComponentId RegisterComponentType(const std::string &name,const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags,std::type_index typeIndex);
		ComponentId RegisterComponentType(const std::string &name,const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags);
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			ComponentId RegisterComponentType(const std::string &name);
		bool GetComponentTypeId(const std::string &name,ComponentId &outId,bool bIncludePreregistered=true) const;
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			bool GetComponentTypeId(ComponentId &outId) const;
		bool GetComponentTypeIndex(ComponentId componentId,std::type_index &typeIndex) const;
		bool GetComponentId(std::type_index typeIndex,ComponentId &componentId) const;
		const std::vector<ComponentInfo> &GetRegisteredComponentTypes() const;
		ComponentMemberIndex RegisterMember(ComponentInfo &componentInfo,ComponentMemberInfo &&memberInfo);

		CallbackHandle AddCreationCallback(ComponentId componentId,const std::function<void(std::reference_wrapper<BaseEntityComponent>)> &onCreate);
		CallbackHandle AddCreationCallback(const std::string &componentName,const std::function<void(std::reference_wrapper<BaseEntityComponent>)> &onCreate);

		const ComponentInfo *GetComponentInfo(ComponentId id) const;
		ComponentInfo *GetComponentInfo(ComponentId id);

		template<class T>
			ComponentEventId RegisterEvent(const std::string &evName,EventInfo::Type type=EventInfo::Type::Broadcast)
		{
			return RegisterEvent(evName,typeid(T),type);
		}
		ComponentEventId RegisterEvent(const std::string &evName,std::type_index typeIndex,EventInfo::Type type=EventInfo::Type::Broadcast);
		ComponentEventId RegisterEventById(const std::string &evName,ComponentId componentId,EventInfo::Type type=EventInfo::Type::Broadcast);
		std::optional<ComponentEventId> FindEventId(ComponentId componentId,const std::string &evName) const;
		std::optional<ComponentEventId> FindEventId(const std::string &componentName,const std::string &evName) const;
		bool GetEventId(const std::string &evName,ComponentEventId &evId) const;
		ComponentEventId GetEventId(const std::string &evName) const;
		bool GetEventName(ComponentEventId evId,std::string &outEvName) const;
		std::string GetEventName(ComponentEventId evId) const;
		const std::unordered_map<ComponentEventId,EventInfo> &GetEvents() const;

		struct DLLNETWORK ComponentContainerInfo
		{
		protected:
			friend EntityComponentManager;
			void Push(BaseEntityComponent &component);
			void Pop(BaseEntityComponent &component);
			std::size_t GetCount() const;
			const std::vector<BaseEntityComponent*> &GetComponents() const;
		private:
			std::queue<std::size_t> m_freeIndices = {};
			std::size_t m_count = 0ull;
			std::vector<BaseEntityComponent*> m_components = {};
		};

		const std::vector<ComponentContainerInfo> &GetComponents() const;
		std::vector<ComponentContainerInfo> &GetComponents();
		// Returns all currently active components of the specified type. Note that some items in the container may be NULL.
		const std::vector<BaseEntityComponent*> &GetComponents(ComponentId componentId) const;
		const std::vector<BaseEntityComponent*> &GetComponents(ComponentId componentId,std::size_t &count) const;

		// Automatically called when a component was removed; Don't call this manually!
		void DeregisterComponent(BaseEntityComponent &component);
	private:
		ComponentId RegisterComponentType(const std::string &name,const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags,const std::type_index *typeIndex);
		virtual void OnComponentTypeRegistered(const ComponentInfo &componentInfo);

		std::vector<ComponentInfo> m_preRegistered;
		std::vector<ComponentInfo> m_componentInfos;
		std::unordered_map<std::type_index,ComponentId> m_typeIndexToComponentId;
		std::vector<std::shared_ptr<std::type_index>> m_componentIdToTypeIndex;
		ComponentId m_nextComponentId = 0u;

		// List of all created components by component id
		mutable std::vector<ComponentContainerInfo> m_components;

		std::unordered_map<ComponentEventId,EventInfo> m_componentEvents;
	};
	using TRegisterComponentEvent = const std::function<ComponentEventId(const std::string&,EntityComponentManager::EventInfo::Type)>&;
	using TRegisterComponentMember = const std::function<ComponentMemberIndex(ComponentMemberInfo&&)>&;
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ComponentFlags);

template<class TComponent,typename>
	pragma::ComponentId pragma::EntityComponentManager::RegisterComponentType(const std::string &name)
{
	auto flags = ComponentFlags::None;
	if(std::is_base_of<pragma::BaseNetComponent,TComponent>::value)
		flags |= ComponentFlags::Networked;
	auto componentId = PreRegisterComponentType(name);
	TComponent::RegisterEvents(*this,[this,componentId](const std::string &evName,EventInfo::Type type) {
		auto id = RegisterEvent<TComponent>(evName,type);
		auto it = m_componentEvents.find(id);
		assert(it != m_componentEvents.end());
		it->second.componentId = componentId;
		return id;
	});
	RegisterComponentType(name,[](BaseEntity &ent) {
		return util::TSharedHandle<BaseEntityComponent>{new TComponent{ent},[](pragma::BaseEntityComponent *c) {delete c;}};
	},flags,std::type_index(typeid(TComponent)));
	auto &componentInfo = m_componentInfos[componentId];
	TComponent::RegisterMembers(*this,[this,&componentInfo](ComponentMemberInfo &&memberInfo) -> ComponentMemberIndex {
		return RegisterMember(componentInfo,std::move(memberInfo));
	});
	return componentId;
}

template<class TComponent,typename>
	util::TSharedHandle<TComponent> pragma::EntityComponentManager::CreateComponent(BaseEntity &ent) const
{
	auto it = std::find(m_componentInfos.begin(),m_componentInfos.end(),std::type_index(typeid(TComponent)));
	if(it == m_componentInfos.end())
		throw std::invalid_argument("Attempted to create unregistered entity component!");
	auto r = it->factory(ent);
	if(r == nullptr)
		return nullptr;
	r->m_componentId = it->id;
	return r;
}

template<class TComponent,typename>
	bool pragma::EntityComponentManager::GetComponentTypeId(ComponentId &outId) const
{
	auto it = m_typeIndexToComponentId.find(std::type_index(typeid(TComponent)));
	if(it == m_typeIndexToComponentId.end())
		return false;
	outId = it->second;
	return true;
}

#endif
