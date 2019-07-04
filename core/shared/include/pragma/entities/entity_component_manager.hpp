#ifndef __ENTITY_COMPONENT_MANAGER_HPP__
#define __ENTITY_COMPONENT_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/entity_component_info.hpp"
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

class BaseEntity;
namespace pragma
{
	class BaseEntityComponent;
	enum class ComponentFlags : uint8_t
	{
		None = 0u,
		Networked = 1u
	};
	class DLLNETWORK BaseNetComponent
	{
	public:
		virtual bool ShouldTransmitNetData() const=0;
	protected:
		BaseNetComponent()=default;
	};
	class DLLNETWORK EntityComponentManager
	{
	public:
		EntityComponentManager()=default;
		EntityComponentManager(const EntityComponentManager&)=delete;
		EntityComponentManager &operator=(const EntityComponentManager&)=delete;
		struct DLLNETWORK ComponentInfo
		{
			std::string name;
			std::function<std::shared_ptr<BaseEntityComponent>(BaseEntity&)> factory = nullptr;
			ComponentId id = std::numeric_limits<uint32_t>::max();
			ComponentFlags flags = ComponentFlags::None;

			bool IsValid() const {return factory != nullptr;}
		};
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

		std::shared_ptr<BaseEntityComponent> CreateComponent(const std::string &name,BaseEntity &ent) const;
		std::shared_ptr<BaseEntityComponent> CreateComponent(ComponentId componentId,BaseEntity &ent) const;
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			std::shared_ptr<TComponent> CreateComponent(BaseEntity &ent) const;
		ComponentId PreRegisterComponentType(const std::string &name);
		ComponentId RegisterComponentType(const std::string &name,const std::function<std::shared_ptr<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags,std::type_index typeIndex);
		ComponentId RegisterComponentType(const std::string &name,const std::function<std::shared_ptr<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags);
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			ComponentId RegisterComponentType(const std::string &name);
		bool GetComponentTypeId(const std::string &name,ComponentId &outId,bool bIncludePreregistered=true) const;
		template<class TComponent,typename=std::enable_if_t<std::is_final<TComponent>::value && std::is_base_of<BaseEntityComponent,TComponent>::value>>
			bool GetComponentTypeId(ComponentId &outId) const;
		bool GetComponentTypeIndex(ComponentId componentId,std::type_index &typeIndex) const;
		bool GetComponentId(std::type_index typeIndex,ComponentId &componentId) const;
		const std::vector<ComponentInfo> &GetRegisteredComponentTypes() const;

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
		ComponentId RegisterComponentType(const std::string &name,const std::function<std::shared_ptr<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags,const std::type_index *typeIndex);
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
	return RegisterComponentType(name,[](BaseEntity &ent) {
		return std::static_pointer_cast<BaseEntityComponent>(std::make_shared<TComponent>(ent));
	},flags,std::type_index(typeid(TComponent)));
}

template<class TComponent,typename>
	std::shared_ptr<TComponent> pragma::EntityComponentManager::CreateComponent(BaseEntity &ent) const
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
