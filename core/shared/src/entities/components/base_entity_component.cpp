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

DLLNETWORK std::ostream& operator<<(std::ostream &os,const pragma::BaseEntityComponent &component)
{
	auto *info = component.GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(component.GetComponentId());
	os<<"Component[";
	if(!info)
		os<<"NULL";
	else
		os<<info->name<<"]["<<info->id<<"]["<<magic_enum::flags::enum_name(info->flags);
	os<<"]";
	return os;
}

decltype(EEntityComponentCallbackEvent::Count) EEntityComponentCallbackEvent::Count = EEntityComponentCallbackEvent{umath::to_integral(E::Count)};
decltype(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED) BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED = INVALID_COMPONENT_ID;
decltype(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED) BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED = INVALID_COMPONENT_ID;
decltype(BaseEntityComponent::EVENT_ON_MEMBERS_CHANGED) BaseEntityComponent::EVENT_ON_MEMBERS_CHANGED = INVALID_COMPONENT_ID;
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
void BaseEntityComponent::RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent)
{
	EVENT_ON_ENTITY_COMPONENT_ADDED = registerEvent("ON_ENTITY_COMPONENT_ADDED",ComponentEventInfo::Type::Broadcast);
	EVENT_ON_ENTITY_COMPONENT_REMOVED = registerEvent("ON_ENTITY_COMPONENT_REMOVED",ComponentEventInfo::Type::Broadcast);
	EVENT_ON_MEMBERS_CHANGED = registerEvent("ON_MEMBERS_CHANGED",ComponentEventInfo::Type::Broadcast);
}
void BaseEntityComponent::Log(const std::string &msg,LogSeverity severity) const
{
	auto print = [&msg](auto &con) {
		con<<msg<<Con::endl;
	};
	switch(severity)
	{
	case LogSeverity::Normal:
	case LogSeverity::Debug:
		print(Con::cout);
		break;
	case LogSeverity::Warning:
		print(Con::cwar);
		break;
	case LogSeverity::Error:
		print(Con::cerr);
		break;
	case LogSeverity::Critical:
		print(Con::crit);
		break;
	}
}
void BaseEntityComponent::OnMembersChanged()
{
	BroadcastEvent(EVENT_ON_MEMBERS_CHANGED);

	auto *genericC = GetEntity().GetGenericComponent();
	if(genericC)
	{
		CEOnMembersChanged ev {*this};
		genericC->InvokeEventCallbacks(BaseGenericComponent::EVENT_ON_MEMBERS_CHANGED,ev);
	}
}
void BaseEntityComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember) {}
const ComponentMemberInfo *BaseEntityComponent::FindMemberInfo(const std::string &name) const
{
	auto idx = GetMemberIndex(name);
	if(!idx.has_value())
		return nullptr;
	return GetMemberInfo(*idx);
}
const ComponentMemberInfo *BaseEntityComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto *componentInfo = GetComponentInfo();
	if(!componentInfo || idx >= componentInfo->members.size())
		return nullptr;
	return &componentInfo->members[idx];
}
const ComponentInfo *BaseEntityComponent::GetComponentInfo() const
{
	return GetEntity().GetComponentManager()->GetComponentInfo(GetComponentId());
}
uint32_t BaseEntityComponent::GetStaticMemberCount() const
{
	auto *componentInfo = GetComponentInfo();
	if(!componentInfo)
		return 0;
	return componentInfo->members.size();
}
std::optional<ComponentMemberIndex> BaseEntityComponent::GetMemberIndex(const std::string &name) const
{
	auto lname = name;
	ustring::to_lower(lname);
	return DoGetMemberIndex(lname);
}
std::optional<ComponentMemberIndex> BaseEntityComponent::DoGetMemberIndex(const std::string &name) const
{
	auto *componentInfo = GetEntity().GetComponentManager()->GetComponentInfo(GetComponentId());
	if(!componentInfo)
		return {};
	auto itMember = componentInfo->memberNameToIndex.find(name);
	if(itMember == componentInfo->memberNameToIndex.end())
		return {};
	return itMember->second;
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
	if(it != events.end() && it->second.typeIndex.has_value() && componentTypeIndex != *it->second.typeIndex && baseTypeIndex != *it->second.typeIndex)
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
	auto &evs = itEv->second;
	for(auto i=decltype(evs.size()){0u};i<evs.size();)
	{
		auto &hCb = evs[i];
		if(hCb.IsValid() == false)
		{
			itEv->second.erase(itEv->second.begin() +i);
			continue;
		}
		if(hCb.Call<util::EventReply,std::reference_wrapper<ComponentEvent>>(std::reference_wrapper<ComponentEvent>(evData)) == util::EventReply::Handled)
			return util::EventReply::Handled;
		if(hThis.expired()) // This component has been removed directly or indirectly by the callback; Return immediately
			return util::EventReply::Unhandled;
		++i;
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
		if(info.typeIndex.has_value())
		{
			for(auto &pComponent : ent.GetComponents())
			{
				auto componentTypeIndex = std::type_index(typeid(*pComponent));
				auto baseTypeIndex = componentTypeIndex;
				pComponent->GetBaseTypeIndex(baseTypeIndex);
				if(componentTypeIndex != *info.typeIndex && baseTypeIndex != *info.typeIndex)
					continue;
				auto cb = pComponent->AddEventCallback(eventId,hCallback);
				FlagCallbackForRemoval(cb,CallbackType::Component,pComponent.get());
				return cb;
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
			if(!info.typeIndex.has_value())
				continue;
			auto componentTypeIndex = std::type_index(typeid(component));
			auto baseTypeIndex = componentTypeIndex;
			component.GetBaseTypeIndex(baseTypeIndex);
			if(componentTypeIndex != *info.typeIndex && baseTypeIndex != *info.typeIndex)
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
		if(!info.typeIndex.has_value())
			continue;
		auto componentTypeIndex = std::type_index(typeid(component));
		auto baseTypeIndex = componentTypeIndex;
		component.GetBaseTypeIndex(baseTypeIndex);
		if(componentTypeIndex != *info.typeIndex && baseTypeIndex != *info.typeIndex)
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
		genericC->InvokeEventCallbacks(BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED,evData);
}
Game &BaseEntityComponent::GetGame() {return *GetNetworkState().GetGameState();}
NetworkState &BaseEntityComponent::GetNetworkState() {return *GetEntity().GetNetworkState();}
EntityComponentManager &BaseEntityComponent::GetComponentManager() {return GetGame().GetEntityComponentManager();}
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
void BaseEntityComponent::OnEntityPostSpawn() {UpdateTickPolicy();}
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
void BaseEntityComponent::UpdateTickPolicy()
{
	if(!GetEntity().IsSpawned())
		return;
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
void BaseEntityComponent::SetTickPolicy(TickPolicy policy)
{
	if(policy == m_tickData.tickPolicy)
		return;
	m_tickData.tickPolicy = policy;

	if(umath::is_flag_set(m_stateFlags,StateFlags::IsThinking))
		return; // Tick policy update will be handled by game
	UpdateTickPolicy();
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

std::string BaseEntityComponent::GetUri() const
{
	auto uri = GetUri(nullptr,GetEntity().GetUuid(),GetComponentInfo()->name);
	assert(uri.has_value());
	return *uri;
}
std::string BaseEntityComponent::GetMemberUri(const std::string &memberName) const
{
	auto uri = GetUri();
	auto q = uri.find('?');
	return uri.substr(0,q) +"/" +memberName +uri.substr(q);
}
std::optional<std::string> BaseEntityComponent::GetMemberUri(ComponentMemberIndex memberIdx) const
{
	auto *info = GetMemberInfo(memberIdx);
	if(!info)
		return {};
	return GetMemberUri(GetEntity().GetNetworkState()->GetGameState(),GetEntity().GetUuid(),GetComponentId(),info->GetName());
}
std::optional<std::string> BaseEntityComponent::GetUri(Game *game,std::variant<util::Uuid,std::string> entityIdentifier,std::variant<ComponentId,std::string> componentIdentifier)
{
	return std::visit([&componentIdentifier,game](auto &value) -> std::optional<std::string> {
		auto uri = BaseEntity::GetUri(value);
		auto q = uri.find('?');
		auto componentName = std::visit([game](auto &value) -> std::optional<std::string> {
			using T = util::base_type<decltype(value)>;
			if constexpr(std::is_same_v<T,ComponentId>)
			{
				if(!game)
					return {};
				auto *info = game->GetEntityComponentManager().GetComponentInfo(value);
				if(!info)
					return {};
				return info->name;
			}
			else
				return value;
		},componentIdentifier);
		if(!componentName.has_value())
			return {};
		return uri.substr(0,q) +"/ec/" +*componentName +uri.substr(q);
	},entityIdentifier);
}
std::optional<std::string> BaseEntityComponent::GetMemberUri(Game *game,std::variant<util::Uuid,std::string> entityIdentifier,std::variant<ComponentId,std::string> componentIdentifier,std::variant<ComponentMemberIndex,std::string> memberIdentifier)
{
	auto uri = GetUri(game,entityIdentifier,componentIdentifier);
	if(!uri.has_value())
		return {};
	auto memberName = std::visit([game,&componentIdentifier](auto &memberId) -> std::optional<std::string> {
		using T = util::base_type<decltype(memberId)>;
		if constexpr(std::is_same_v<T,ComponentMemberIndex>)
		{
			if(!game)
				return {};
			return std::visit([game,&memberId](auto &value) -> std::optional<std::string> {
				using T = util::base_type<decltype(value)>;
				ComponentId componentId;
				if constexpr(std::is_same_v<T,ComponentId>)
					componentId = value;
				else
				{
					if(game->GetEntityComponentManager().GetComponentTypeId(value,componentId) == false)
						return {};
				}

				auto *info = game->GetEntityComponentManager().GetComponentInfo(componentId);
				if(!info || memberId >= info->members.size())
					return {};
				auto &memberInfo = info->members[memberId];
				return memberInfo.GetName();
			},componentIdentifier);
		}
		else
			return memberId;
	},memberIdentifier);
	if(!memberName.has_value())
		return {};
	auto q = uri->find('?');
	return uri->substr(0,q) +"/" +*memberName +uri->substr(q);
}
