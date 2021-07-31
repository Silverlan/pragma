/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/components/base_generic_component.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

decltype(EEntityComponentCallbackEvent::Count) EEntityComponentCallbackEvent::Count = EEntityComponentCallbackEvent{umath::to_integral(E::Count)};
decltype(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED) BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED = INVALID_COMPONENT_ID;
decltype(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED) BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED = INVALID_COMPONENT_ID;
BaseEntityComponent::BaseEntityComponent(BaseEntity &ent)
	: m_entity{ent}
{}
BaseEntityComponent::~BaseEntityComponent()
{
	for(auto &cbInfo : m_callbackInfos)
	{
		if(cbInfo.hCallback.IsValid() == false)
			continue;
		cbInfo.hCallback.Remove();
	}
	GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().DeregisterComponent(*this);
}
void BaseEntityComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_ENTITY_COMPONENT_ADDED = componentManager.RegisterEvent("ON_ENTITY_COMPONENT_ADDED");
	EVENT_ON_ENTITY_COMPONENT_REMOVED = componentManager.RegisterEvent("ON_ENTITY_COMPONENT_REMOVED");
}
util::TWeakSharedHandle<const BaseEntityComponent> BaseEntityComponent::GetHandle() const {return GetHandle<BaseEntityComponent>();}
util::TWeakSharedHandle<BaseEntityComponent> BaseEntityComponent::GetHandle() {return GetHandle<BaseEntityComponent>();}
void BaseEntityComponent::Initialize()
{
	InitializeLuaObject(GetEntity().GetLuaState());

	auto &ent = GetEntity();
	OnAttached(ent);
	if(ent.IsSpawned())
	{
		OnEntitySpawn();
		OnEntityPostSpawn();
	}
}
void BaseEntityComponent::OnRemove()
{
	OnDetached(GetEntity());
	for(auto &pair : m_eventCallbacks)
	{
		for(auto &hCb : pair.second)
		{
			if(hCb.IsValid() == false)
				continue;
			hCb.Remove();
		}
	}
	for(auto &pair : m_boundEvents)
	{
		for(auto &hCb : pair.second)
		{
			if(hCb.IsValid() == false)
				continue;
			hCb.Remove();
		}
	}
	if(umath::is_flag_set(m_stateFlags,StateFlags::IsLogicEnabled))
	{
		auto &logicComponents = GetEntity().GetNetworkState()->GetGameState()->GetEntityTickComponents();
		*std::find(logicComponents.begin(),logicComponents.end(),this) = nullptr;
		umath::set_flag(m_stateFlags,StateFlags::IsLogicEnabled,false);
	}
}
bool BaseEntityComponent::ShouldTransmitNetData() const {return false;}
bool BaseEntityComponent::ShouldTransmitSnapshotData() const {return false;}
const luabind::object &BaseEntityComponent::GetLuaObject() const {return const_cast<BaseEntityComponent*>(this)->GetLuaObject();}
luabind::object &BaseEntityComponent::GetLuaObject() {return m_luaObj;}
lua_State *BaseEntityComponent::GetLuaState() const {return m_luaObj.interpreter();}
void BaseEntityComponent::PushLuaObject(lua_State *l) {GetLuaObject().push(l);}
void BaseEntityComponent::PushLuaObject() {PushLuaObject(GetLuaObject().interpreter());}
void BaseEntityComponent::FlagCallbackForRemoval(const CallbackHandle &hCallback,CallbackType cbType,BaseEntityComponent *component)
{
	switch(cbType)
	{
		case CallbackType::Entity:
			component = nullptr;
			break;
		case CallbackType::Component:
			if(component == nullptr)
				component = this;
			break;
	}
	m_callbackInfos.push_back({});
	auto &cbInfo = m_callbackInfos.back();
	cbInfo.hCallback = hCallback;
	cbInfo.pComponent = component;
}
const BaseEntity &BaseEntityComponent::GetEntity() const {return const_cast<BaseEntityComponent*>(this)->GetEntity();}
BaseEntity &BaseEntityComponent::GetEntity() {return m_entity;}
const BaseEntity &BaseEntityComponent::operator->() const {return GetEntity();}
BaseEntity &BaseEntityComponent::operator->() {return GetEntity();}
ComponentId BaseEntityComponent::GetComponentId() const {return m_componentId;}
CallbackHandle BaseEntityComponent::AddEventCallback(ComponentEventId eventId,const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback)
{
	return AddEventCallback(eventId,FunctionCallback<util::EventReply,std::reference_wrapper<ComponentEvent>>::Create(fCallback));
}
CallbackHandle BaseEntityComponent::AddEventCallback(ComponentEventId eventId,const CallbackHandle &hCallback)
{
	// Sanity check (to make sure the event type is actually associated with this component)
	auto componentTypeIndex = std::type_index(typeid(*this));
	auto baseTypeIndex = componentTypeIndex;
	GetBaseTypeIndex(baseTypeIndex);
	auto &events = GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
	auto it = events.find(eventId);
	if(it != events.end() && it->second.componentType != nullptr && componentTypeIndex != *it->second.componentType && baseTypeIndex != *it->second.componentType)
		throw std::logic_error("Attempted to add callback for component event " +std::to_string(eventId) +" (" +it->second.name +") to component " +std::string(typeid(*this).name()) +", which this event does not belong to!");
	
	auto itEv = m_eventCallbacks.find(eventId);
	if(itEv == m_eventCallbacks.end())
		itEv = m_eventCallbacks.insert(std::make_pair(eventId,std::vector<CallbackHandle>{})).first;
	itEv->second.push_back(hCallback);
	return itEv->second.back();
}
void BaseEntityComponent::RemoveEventCallback(ComponentEventId eventId,const CallbackHandle &hCallback)
{
	auto itEv = m_eventCallbacks.find(eventId);
	if(itEv == m_eventCallbacks.end())
		return;
	auto itCb = std::find(itEv->second.begin(),itEv->second.end(),hCallback);
	if(itCb == itEv->second.end())
		return;
	itEv->second.erase(itCb);
	if(itEv->second.empty())
		m_eventCallbacks.erase(itEv);
}
util::EventReply BaseEntityComponent::InvokeEventCallbacks(ComponentEventId eventId,const ComponentEvent &evData) const
{
	return InvokeEventCallbacks(eventId,const_cast<ComponentEvent&>(evData)); // Hack: This assumes the argument was passed as temporary variable and changing it does not matter
}
util::EventReply BaseEntityComponent::InvokeEventCallbacks(ComponentEventId eventId,ComponentEvent &evData) const
{
	auto itEv = m_eventCallbacks.find(eventId);
	if(itEv == m_eventCallbacks.end())
		return util::EventReply::Unhandled;
	auto hThis = GetHandle();
	for(auto it=itEv->second.begin();it!=itEv->second.end();)
	{
		auto &hCb = *it;
		if(hCb.IsValid() == false)
		{
			it = itEv->second.erase(it);
			continue;
		}
		if(hCb.Call<util::EventReply,std::reference_wrapper<ComponentEvent>>(std::reference_wrapper<ComponentEvent>(evData)) == util::EventReply::Handled)
			return util::EventReply::Handled;
		if(hThis.expired()) // This component has been removed directly or indirectly by the callback; Return immediately
			return util::EventReply::Unhandled;
		++it;
	}
	return util::EventReply::Unhandled;
}
util::EventReply BaseEntityComponent::InvokeEventCallbacks(ComponentEventId eventId) const
{
	CEGenericComponentEvent ev {};
	return InvokeEventCallbacks(eventId,ev);
}
util::EventReply BaseEntityComponent::BroadcastEvent(ComponentEventId eventId,const ComponentEvent &evData) const
{
	return BroadcastEvent(eventId,const_cast<ComponentEvent&>(evData)); // Hack: This assumes the argument was passed as temporary variable and changing it does not matter
}
util::EventReply BaseEntityComponent::BroadcastEvent(ComponentEventId eventId,ComponentEvent &evData) const
{
	auto &ent = GetEntity();
	if(ent.BroadcastEvent(eventId,evData,this) == util::EventReply::Handled)
		return util::EventReply::Handled;
	return InvokeEventCallbacks(eventId,evData);
}
util::EventReply BaseEntityComponent::BroadcastEvent(ComponentEventId eventId) const
{
	CEGenericComponentEvent ev {};
	return BroadcastEvent(eventId,ev);
}
util::EventReply BaseEntityComponent::InjectEvent(ComponentEventId eventId,const ComponentEvent &evData) {return InjectEvent(eventId,const_cast<ComponentEvent&>(evData));}
util::EventReply BaseEntityComponent::InjectEvent(ComponentEventId eventId,ComponentEvent &evData) {return HandleEvent(eventId,evData);}
util::EventReply BaseEntityComponent::InjectEvent(ComponentEventId eventId)
{
	CEGenericComponentEvent ev {};
	return BroadcastEvent(eventId,ev);
}
CallbackHandle BaseEntityComponent::BindEventUnhandled(ComponentEventId eventId,const std::function<void(std::reference_wrapper<ComponentEvent>)> &fCallback)
{
	return BindEvent(eventId,[fCallback](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		fCallback(evData);
		return util::EventReply::Unhandled;
	});
}
CallbackHandle BaseEntityComponent::BindEvent(ComponentEventId eventId,const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback)
{
	auto hCallback = FunctionCallback<util::EventReply,std::reference_wrapper<ComponentEvent>>::Create(fCallback);
	auto &ent = GetEntity();
	auto &events = ent.GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
	auto itInfo = events.find(eventId);
	if(itInfo != events.end())
	{
		auto &info = itInfo->second;
		if(info.componentType != nullptr)
		{
			for(auto &pComponent : ent.GetComponents())
			{
				auto componentTypeIndex = std::type_index(typeid(*pComponent));
				auto baseTypeIndex = componentTypeIndex;
				pComponent->GetBaseTypeIndex(baseTypeIndex);
				if(componentTypeIndex != *info.componentType && baseTypeIndex != *info.componentType)
					continue;
				pComponent->AddEventCallback(eventId,hCallback);
			}
		}
	}
	auto itEv = m_boundEvents.find(eventId);
	if(itEv == m_boundEvents.end())
		itEv = m_boundEvents.insert(std::make_pair(eventId,std::vector<CallbackHandle>{})).first;
	itEv->second.push_back(hCallback);
	return itEv->second.back();
}
util::EventReply BaseEntityComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(eventId == BaseEntity::EVENT_ON_SPAWN)
		OnEntitySpawn();
	else if(eventId == BaseEntity::EVENT_ON_POST_SPAWN)
		OnEntityPostSpawn();

	auto itEv = m_boundEvents.find(eventId);
	if(itEv == m_boundEvents.end())
		return util::EventReply::Unhandled;
	for(auto it=itEv->second.begin();it!=itEv->second.end();)
	{
		auto &hCb = *it;
		if(hCb.IsValid() == false)
		{
			it = itEv->second.erase(it);
			continue;
		}
		if(hCb.Call<util::EventReply,std::reference_wrapper<ComponentEvent>>(std::reference_wrapper<ComponentEvent>(evData)) == util::EventReply::Handled)
			return util::EventReply::Handled;
		++it;
	}
	return util::EventReply::Unhandled;
}
void BaseEntityComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const {}
void BaseEntityComponent::OnEntityComponentAdded(BaseEntityComponent &component) {}
void BaseEntityComponent::OnEntityComponentAdded(BaseEntityComponent &component,bool bSkipEventBinding)
{
	if(bSkipEventBinding == false)
	{
		auto &events = GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
		for(auto &pair : m_boundEvents)
		{
			auto evId = pair.first;
			auto &info = events.at(evId);
			if(info.componentType == nullptr)
				continue;
			auto componentTypeIndex = std::type_index(typeid(component));
			auto baseTypeIndex = componentTypeIndex;
			component.GetBaseTypeIndex(baseTypeIndex);
			if(componentTypeIndex != *info.componentType && baseTypeIndex != *info.componentType)
				continue;
			for(auto &hCb : pair.second)
				component.AddEventCallback(evId,hCb);
		}
	}
	OnEntityComponentAdded(component);
}
void BaseEntityComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	auto &events = GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
	for(auto &pair : m_boundEvents)
	{
		auto evId = pair.first;
		auto &info = events.at(evId);
		if(info.componentType == nullptr)
			continue;
		auto componentTypeIndex = std::type_index(typeid(component));
		auto baseTypeIndex = componentTypeIndex;
		component.GetBaseTypeIndex(baseTypeIndex);
		if(componentTypeIndex != *info.componentType && baseTypeIndex != *info.componentType)
			continue;
		for(auto &hCb : pair.second)
			component.RemoveEventCallback(evId,hCb);
	}
	for(auto it=m_callbackInfos.begin();it!=m_callbackInfos.end();)
	{
		auto &cbInfo = *it;
		if(cbInfo.pComponent != &component && cbInfo.hCallback.IsValid())
		{
			++it;
			continue;
		}
		if(cbInfo.hCallback.IsValid())
			it->hCallback.Remove();
		it = m_callbackInfos.erase(it);
	}
	pragma::CEOnEntityComponentRemoved evData{*this};
	auto *genericC = GetEntity().GetGenericComponent();
	if(BroadcastEvent(EVENT_ON_ENTITY_COMPONENT_REMOVED,evData) != util::EventReply::Handled && genericC)
		genericC->InvokeEventCallbacks(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED,evData);
}
void BaseEntityComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	udm["version"] = GetVersion();

	auto tCur = GetEntity().GetNetworkState()->GetGameState()->CurTime();
	udm["lastTick"] = m_tickData.lastTick -tCur;
	udm["nextTick"] = m_tickData.nextTick -tCur;
}
void BaseEntityComponent::Load(udm::LinkedPropertyWrapperArg udm)
{
	uint32_t version = 0;
	udm["version"](version);

	float lastTick = 0.f;
	float nextTick = 0.f;
	udm["lastTick"](lastTick);
	udm["nextTick"](nextTick);

	auto tCur = GetEntity().GetNetworkState()->GetGameState()->CurTime();
	m_tickData.lastTick += tCur;
	m_tickData.nextTick += tCur;
	Load(udm,version);
}
void BaseEntityComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) {}
void BaseEntityComponent::OnEntitySpawn() {}
void BaseEntityComponent::OnEntityPostSpawn() {}
void BaseEntityComponent::OnAttached(BaseEntity &ent) {}
void BaseEntityComponent::OnDetached(BaseEntity &ent)
{
	for(auto it=m_callbackInfos.begin();it!=m_callbackInfos.end();)
	{
		auto &cbInfo = *it;
		if(cbInfo.pComponent != nullptr && cbInfo.hCallback.IsValid())
		{
			++it;
			continue;
		}
		if(cbInfo.hCallback.IsValid())
			it->hCallback.Remove();
		it = m_callbackInfos.erase(it);
	}
}
pragma::NetEventId BaseEntityComponent::SetupNetEvent(const std::string &name) const {return GetEntity().GetNetworkState()->GetGameState()->SetupNetEvent(name);}

//////////////////

TickPolicy BaseEntityComponent::GetTickPolicy() const {return m_tickData.tickPolicy;}

bool BaseEntityComponent::ShouldThink() const
{
	if(m_tickData.tickPolicy != TickPolicy::Always && m_tickData.tickPolicy != TickPolicy::WhenVisible)
		return false;
	//auto toggleC = static_cast<pragma::BaseToggleComponent*>(GetEntity().FindComponent("toggle").get());
	//return toggleC ? toggleC->IsTurnedOn() : true;
	return true;
}
void BaseEntityComponent::SetTickPolicy(TickPolicy policy)
{
	if(policy == m_tickData.tickPolicy)
		return;
	m_tickData.tickPolicy = policy;

	if(umath::is_flag_set(m_stateFlags,StateFlags::IsThinking))
		return; // Tick policy update will be handled by game
	
	auto &logicComponents = GetEntity().GetNetworkState()->GetGameState()->GetEntityTickComponents();
	if(ShouldThink())
	{
		if(umath::is_flag_set(m_stateFlags,StateFlags::IsLogicEnabled))
			return;
		logicComponents.push_back(this);
		umath::set_flag(m_stateFlags,StateFlags::IsLogicEnabled);
		return;
	}
	if(!umath::is_flag_set(m_stateFlags,StateFlags::IsLogicEnabled))
		return;
	logicComponents.erase(std::find(logicComponents.begin(),logicComponents.end(),this));
	umath::set_flag(m_stateFlags,StateFlags::IsLogicEnabled,false);
}

double BaseEntityComponent::GetNextTick() const {return m_tickData.nextTick;}
void BaseEntityComponent::SetNextTick(double t) {m_tickData.nextTick = t;}

double BaseEntityComponent::LastTick() const {return m_tickData.lastTick;}

double BaseEntityComponent::DeltaTime() const
{
	Game *game = GetEntity().GetNetworkState()->GetGameState();
	//auto r = game->CurTime() -m_lastThink; // This would be more accurate, but can be 0 if the engine had to catch up on the tick rate
	auto r = game->DeltaTickTime();
	//assert(r != 0.0); // Delta time mustn't ever be 0, otherwise there can be problems with animation events repeating (among other things)
	return r;
}

bool BaseEntityComponent::Tick(double tDelta)
{
	m_stateFlags |= pragma::BaseEntityComponent::StateFlags::IsThinking;

	auto hThis = GetHandle();
	auto &ent = GetEntity();
	OnTick(tDelta);
	if(hThis.expired())
		return true; // This component isn't valid anymore; Return immediately
	Game *game = ent.GetNetworkState()->GetGameState();
	m_tickData.lastTick = game->CurTime();

	m_stateFlags &= ~pragma::BaseEntityComponent::StateFlags::IsThinking;

	if(ShouldThink() == false)
	{
		m_stateFlags &= ~pragma::BaseEntityComponent::StateFlags::IsLogicEnabled;
		return false; // Game will handle removal from tick componentlist
	}
	return true;
}
