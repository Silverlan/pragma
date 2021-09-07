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

namespace udm {enum class Type : uint8_t;};

class BaseEntity;
namespace pragma
{
	class BaseEntityComponent;
	DLLNETWORK std::string get_normalized_component_member_name(const std::string &name);
	DLLNETWORK size_t get_component_member_name_hash(const std::string &name);
	struct DLLNETWORK ComponentMemberInfo
	{
		using ApplyFunction = void(*)(const ComponentMemberInfo&,BaseEntityComponent&,const void*);
		using GetFunction = void(*)(const ComponentMemberInfo&,BaseEntityComponent&,void*);
		using InterpolationFunction = void(*)(const void*,const void*,double,void*);
		static ComponentMemberInfo CreateDummy();
		ComponentMemberInfo(std::string &&name,udm::Type type,const ApplyFunction &applyFunc,const GetFunction &getFunc);

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

		udm::Type type;
		ApplyFunction setterFunction = nullptr;
		GetFunction getterFunction = nullptr;
		InterpolationFunction interpolationFunction = nullptr;
		union
		{
			uint64_t userIndex;
			void *userData = nullptr;
		};
	private:
		ComponentMemberInfo()=default;
		std::string m_name;
		size_t m_nameHash = 0;
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
		std::string name;
		std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> factory = nullptr;
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
			EventInfo(const std::string &name)
				: name(name)
			{}
			EventInfo(const std::string &name,std::type_index typeId)
				: name(name),componentType(std::make_unique<std::type_index>(typeId))
			{}
			std::string name;
			// If componentType is set, this event can only be caught by
			// using AddEventCallback on the component of this type
			std::unique_ptr<std::type_index> componentType = nullptr;
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

		const ComponentInfo *GetComponentInfo(ComponentId id) const;
		ComponentInfo *GetComponentInfo(ComponentId id);

		ComponentEventId RegisterEvent(const std::string &evName,std::type_index componentType);
		ComponentEventId RegisterEvent(const std::string &evName);
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

		// These HAVE to be static, since event ids are usually static class variables,
		// and have to be the same on the client and server.
		static ComponentEventId s_nextEventId;
		static std::unordered_map<ComponentEventId,EventInfo> s_componentEvents;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ComponentFlags);

template<class TComponent,typename>
	pragma::ComponentId pragma::EntityComponentManager::RegisterComponentType(const std::string &name)
{
	auto flags = ComponentFlags::None;
	if(std::is_base_of<pragma::BaseNetComponent,TComponent>::value)
		flags |= ComponentFlags::Networked;
	TComponent::RegisterEvents(*this);
	auto id = RegisterComponentType(name,[](BaseEntity &ent) {
		return util::TSharedHandle<BaseEntityComponent>{new TComponent{ent},[](pragma::BaseEntityComponent *c) {delete c;}};
	},flags,std::type_index(typeid(TComponent)));
	auto &componentInfo = m_componentInfos[id];
	TComponent::RegisterMembers(*this,[this,&componentInfo](ComponentMemberInfo &&memberInfo) -> ComponentMemberIndex {
		return RegisterMember(componentInfo,std::move(memberInfo));
	});
	return id;
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
