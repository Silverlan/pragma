/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/components/base_entity_component.hpp"

using namespace pragma;

decltype(EntityComponentManager::s_nextEventId) EntityComponentManager::s_nextEventId = 0u;
decltype(EntityComponentManager::s_componentEvents) EntityComponentManager::s_componentEvents = {};

std::optional<ComponentMemberIndex> EntityComponentManager::ComponentInfo::FindMember(const std::string &name) const
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = std::find_if(members.begin(),members.end(),[&lname](const ComponentMemberInfo &memberInfo) {
		return memberInfo.name == lname;
	});
	return (it != members.end()) ? (it -members.begin()) : std::numeric_limits<ComponentMemberIndex>::max();
}

util::TSharedHandle<BaseEntityComponent> EntityComponentManager::CreateComponent(ComponentId componentId,BaseEntity &ent) const
{
	if(componentId >= m_componentInfos.size() || m_componentInfos[componentId].id == INVALID_COMPONENT_ID)
	{
		// Component has been pre-registered, but its script has not yet been loaded!
		// 'info'-members will not be valid, so we have to retrieve the component information
		// (name, etc.) from the pre-register data
		auto it = std::find_if(m_preRegistered.begin(),m_preRegistered.end(),[componentId](const ComponentInfo &componentInfo) {
			return componentInfo.id == componentId;
		});
		if(it == m_preRegistered.end())
		{
			// Component has NOT been pre-registered? Then where did the component id come from?
			// This should be unreachable!
			Con::cerr<<"ERROR: Attempted to create unknown component '"<<componentId<<"'!"<<Con::endl;
			return nullptr;
		}
		auto &preRegInfo = *it;
		// Attempt to create the component by name instead.
		// This will automatically attempt to load the appropriate
		// component script.
		auto name = preRegInfo.name; // Name has to be copied, because pre-register information may be invalidated by the 'CreateComponent'-call
		return CreateComponent(name,ent);
	}
	auto &info = m_componentInfos.at(componentId);
	auto r = info.factory(ent);
	if(r == nullptr)
		return nullptr;
	r->m_componentId = info.id;
	m_components.at(r->m_componentId).Push(*r);
	return r;
}
util::TSharedHandle<BaseEntityComponent> EntityComponentManager::CreateComponent(const std::string &name,BaseEntity &ent) const
{
	auto componentId = pragma::INVALID_COMPONENT_ID;
	// Try to find the component as Lua component and load it (if the component hasn't been registered yet)
	if(
		GetComponentTypeId(name,componentId,false) == false &&
		(ent.GetNetworkState()->GetGameState()->LoadLuaComponentByName(name) == false ||
		GetComponentTypeId(name,componentId,false) == false)
	)
		return nullptr;
	auto *componentInfo = GetComponentInfo(componentId);
	if(componentInfo == nullptr || componentInfo->IsValid() == false)
		return nullptr;
	auto r = componentInfo->factory(ent);
	if(r == nullptr)
		return nullptr;
	r->m_componentId = componentInfo->id;
	m_components.at(r->m_componentId).Push(*r);
	return r;
}
ComponentId EntityComponentManager::PreRegisterComponentType(const std::string &name)
{
	auto componentId = pragma::INVALID_COMPONENT_ID;
	if(GetComponentTypeId(name,componentId))
		return componentId;
	m_preRegistered.push_back({});
	auto &componentInfo = m_preRegistered.back();
	componentInfo.name = name;
	componentInfo.id = m_nextComponentId++;
	ustring::to_lower(componentInfo.name);
	if(componentInfo.id != m_components.size())
		throw std::logic_error("Newly registered component id does not match expected component type count!");
	if(m_components.size() == m_components.capacity())
		m_components.reserve(m_components.size() +50);
	m_components.push_back({});
	return componentInfo.id;
}
ComponentId EntityComponentManager::RegisterComponentType(const std::string &name,const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags,std::type_index typeIndex)
{
	return RegisterComponentType(name,factory,flags,&typeIndex);
}
ComponentId EntityComponentManager::RegisterComponentType(const std::string &name,const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags)
{
	return RegisterComponentType(name,factory,flags,nullptr);
}
ComponentId EntityComponentManager::RegisterComponentType(const std::string &name,const std::function<util::TSharedHandle<BaseEntityComponent>(BaseEntity&)> &factory,ComponentFlags flags,const std::type_index *typeIndex)
{
	if(typeIndex != nullptr)
	{
		auto it = m_typeIndexToComponentId.find(*typeIndex);
		if(it != m_typeIndexToComponentId.end())
			throw std::runtime_error("Attempted to register component '" +name +"' which has already been registered previously!");
	}
	auto componentId = pragma::INVALID_COMPONENT_ID;
	if(GetComponentTypeId(name,componentId,false))
	{
		// Overwrite previous definition
		auto &pComponentInfo = *GetComponentInfo(componentId);
		pComponentInfo.factory = factory;
		pComponentInfo.flags = flags;
		OnComponentTypeRegistered(pComponentInfo);
		return componentId;
	}
	auto idx = PreRegisterComponentType(name);
	auto itPre = std::find_if(m_preRegistered.begin(),m_preRegistered.end(),[&name](const ComponentInfo &componentInfo) {
		return ustring::compare(name,componentInfo.name,false);
	});
	if(itPre == m_preRegistered.end())
		throw std::logic_error("Error when attempting to pre-register component " +name +"!");
	componentId = itPre->id;
	if(componentId >= m_componentInfos.size())
		m_componentInfos.resize(componentId +1u,ComponentInfo{});
	auto &componentInfo = m_componentInfos.at(componentId) = *itPre;
	if(typeIndex != nullptr)
	{
		m_typeIndexToComponentId.insert(std::make_pair(*typeIndex,componentId));
		if(componentId >= m_componentIdToTypeIndex.size())
			m_componentIdToTypeIndex.resize(componentId +1u,nullptr);
		m_componentIdToTypeIndex.at(componentId) = std::make_shared<std::type_index>(*typeIndex);
	}
	m_preRegistered.erase(itPre);

	componentInfo.factory = factory;
	componentInfo.flags = flags;
	OnComponentTypeRegistered(componentInfo);
	return componentInfo.id;
}
bool EntityComponentManager::GetComponentTypeIndex(ComponentId componentId,std::type_index &typeIndex) const
{
	if(componentId >= m_componentIdToTypeIndex.size())
		return false;
	auto &pTypeIndex = m_componentIdToTypeIndex.at(componentId);
	if(pTypeIndex == nullptr)
		return false;
	typeIndex = *pTypeIndex;
	return true;
}
bool EntityComponentManager::GetComponentId(std::type_index typeIndex,ComponentId &componentId) const
{
	auto it = m_typeIndexToComponentId.find(typeIndex);
	if(it == m_typeIndexToComponentId.end())
		return false;
	componentId = it->second;
	return true;
}
bool EntityComponentManager::GetComponentTypeId(const std::string &name,ComponentId &outId,bool bIncludePreregistered) const
{
	if(bIncludePreregistered == true)
	{
		auto itPre = std::find_if(m_preRegistered.begin(),m_preRegistered.end(),[&name](const ComponentInfo &componentInfo) {
			return ustring::compare(name,componentInfo.name);
		});
		if(itPre != m_preRegistered.end())
		{
			outId = itPre->id;
			return true;
		}
	}
	auto it = std::find_if(m_componentInfos.begin(),m_componentInfos.end(),[&name](const ComponentInfo &componentInfo) {
		return ustring::compare(name,componentInfo.name,false);
	});
	if(it == m_componentInfos.end())
		return false;
	outId = it->id;
	return true;
}
const EntityComponentManager::ComponentInfo *EntityComponentManager::GetComponentInfo(ComponentId id) const {return const_cast<EntityComponentManager*>(this)->GetComponentInfo(id);}
EntityComponentManager::ComponentInfo *EntityComponentManager::GetComponentInfo(ComponentId id)
{
	if(id >= m_componentInfos.size())
		return nullptr;
	return &m_componentInfos.at(id);
}
ComponentMemberIndex EntityComponentManager::RegisterMember(ComponentInfo &componentInfo,ComponentMemberInfo &&memberInfo)
{
	auto lname = memberInfo.name;
	ustring::to_lower(lname);
	componentInfo.members.push_back(std::move(memberInfo));
	auto idx = componentInfo.members.size() -1;
	componentInfo.memberNameToIndex[lname] = idx;
	return idx;
}
const std::vector<EntityComponentManager::ComponentInfo> &EntityComponentManager::GetRegisteredComponentTypes() const {return m_componentInfos;}
void EntityComponentManager::OnComponentTypeRegistered(const ComponentInfo &componentInfo) {}
ComponentEventId EntityComponentManager::RegisterEvent(const std::string &evName,std::type_index componentType)
{
	auto it = std::find_if(s_componentEvents.begin(),s_componentEvents.end(),[&evName](const std::pair<const ComponentEventId,EventInfo> &pair) {
		return evName == pair.second.name;
	});
	if(it == s_componentEvents.end())
		it = s_componentEvents.insert(std::make_pair(s_nextEventId++,EventInfo{evName,componentType})).first;
	return it->first;
}
ComponentEventId EntityComponentManager::RegisterEvent(const std::string &evName)
{
	auto it = std::find_if(s_componentEvents.begin(),s_componentEvents.end(),[&evName](const std::pair<const ComponentEventId,EventInfo> &pair) {
		return evName == pair.second.name;
	});
	if(it == s_componentEvents.end())
		it = s_componentEvents.insert(std::make_pair(s_nextEventId++,EventInfo{evName})).first;
	return it->first;
}
bool EntityComponentManager::GetEventId(const std::string &evName,ComponentEventId &evId) const
{
	auto it = std::find_if(s_componentEvents.begin(),s_componentEvents.end(),[&evName](const std::pair<const ComponentEventId,EventInfo> &pair) {
		return evName == pair.second.name;
	});
	if(it == s_componentEvents.end())
		return false;
	evId = it->first;
	return true;
}
pragma::ComponentEventId EntityComponentManager::GetEventId(const std::string &evName) const
{
	ComponentEventId evId;
	if(GetEventId(evName,evId) == false)
		throw std::logic_error("Entity component event '" +evName +"' has not been registered!");
	return evId;
}
bool EntityComponentManager::GetEventName(ComponentEventId evId,std::string &outEvName) const
{
	auto it = s_componentEvents.find(evId);
	if(it == s_componentEvents.end())
		return false;
	outEvName = it->second.name;
	return true;
}
std::string EntityComponentManager::GetEventName(ComponentEventId evId) const
{
	std::string name;
	if(GetEventName(evId,name) == false)
		throw std::logic_error("Entity component event '" +std::to_string(evId) +"' has not been registered!");
	return name;
}
const std::unordered_map<pragma::ComponentEventId,EntityComponentManager::EventInfo> &EntityComponentManager::GetEvents() const {return s_componentEvents;}
const std::vector<EntityComponentManager::ComponentContainerInfo> &EntityComponentManager::GetComponents() const {return const_cast<EntityComponentManager*>(this)->GetComponents();}
std::vector<EntityComponentManager::ComponentContainerInfo> &EntityComponentManager::GetComponents() {return m_components;}
const std::vector<BaseEntityComponent*> &EntityComponentManager::GetComponents(ComponentId componentId) const
{
	std::size_t count;
	return GetComponents(componentId,count);
}
const std::vector<BaseEntityComponent*> &EntityComponentManager::GetComponents(ComponentId componentId,std::size_t &count) const
{
	if(componentId >= m_components.size())
	{
		static std::vector<BaseEntityComponent*> empty {};
		count = 0ull;
		return empty;
	}
	auto &info = m_components.at(componentId);
	count = info.GetCount();
	return info.GetComponents();
}
void EntityComponentManager::DeregisterComponent(BaseEntityComponent &component)
{
	m_components.at(component.GetComponentId()).Pop(component);
}

////////////////////

void EntityComponentManager::ComponentContainerInfo::Push(BaseEntityComponent &component)
{
	if(m_freeIndices.empty() == false)
	{
		auto idx = m_freeIndices.front();
		m_freeIndices.pop();
		while(idx >= m_count && m_freeIndices.empty() == false)
		{
			idx = m_freeIndices.front();
			m_freeIndices.pop();
		}
		if(idx >= m_count)
		{
			Push(component);
			return;
		}
		m_components.at(idx) = &component;
		return;
	}
	if(m_count == m_components.capacity())
		m_components.reserve(umath::max(umath::ceil(m_components.capacity() *1.4f),50)); // Increase capacity to 140% of current capacity
	if(m_count < m_components.size())
	{
		m_components.at(m_count++) = &component;
		return;
	}
	m_components.push_back(&component);
	++m_count;
}
void EntityComponentManager::ComponentContainerInfo::Pop(BaseEntityComponent &component)
{
	auto it = std::find_if(m_components.begin(),m_components.end(),[&component](const pragma::BaseEntityComponent *componentOther) {
		return &component == componentOther;
	});
	if(it == m_components.end())
		return;
	auto idx = it -m_components.begin();
	m_components.at(idx) = nullptr;
	if((idx +1u) == m_count)
	{
		while(idx > 0ull && m_components.at(idx) == nullptr)
		{
			--m_count;
			--idx;
		}
		if(idx == 0ull && m_components.front() == nullptr)
			m_count = 0ull;
	}
	else
		m_freeIndices.push(idx);
}
const std::vector<BaseEntityComponent*> &EntityComponentManager::ComponentContainerInfo::GetComponents() const {return m_components;}
std::size_t EntityComponentManager::ComponentContainerInfo::GetCount() const {return m_count;}

////////////////////

ComponentMemberInfo pragma::ComponentMemberInfo::CreateDummy()
{
	return ComponentMemberInfo{};
}
pragma::ComponentMemberInfo::ComponentMemberInfo(std::string &&name,udm::Type type,const ApplyFunction &applyFunc,const GetFunction &getFunc)
	: name{std::move(name)},type{type},setterFunction{applyFunc},getterFunction{getFunc}
{}
