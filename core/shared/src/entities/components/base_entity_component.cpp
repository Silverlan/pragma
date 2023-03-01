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
#include "pragma/entities/components/origin_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

DLLNETWORK std::ostream &operator<<(std::ostream &os, const pragma::BaseEntityComponent &component)
{
	auto *info = component.GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(component.GetComponentId());
	os << "Component[";
	if(!info)
		os << "NULL";
	else
		os << info->name << "][" << info->id << "][" << magic_enum::flags::enum_name(info->flags);
	os << "]";
	return os;
}

decltype(EEntityComponentCallbackEvent::Count) EEntityComponentCallbackEvent::Count = EEntityComponentCallbackEvent {umath::to_integral(E::Count)};
decltype(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED) BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED = INVALID_COMPONENT_ID;
decltype(BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED) BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED = INVALID_COMPONENT_ID;
decltype(BaseEntityComponent::EVENT_ON_MEMBERS_CHANGED) BaseEntityComponent::EVENT_ON_MEMBERS_CHANGED = INVALID_COMPONENT_ID;
BaseEntityComponent::BaseEntityComponent(BaseEntity &ent) : m_entity {ent} {}
BaseEntityComponent::~BaseEntityComponent()
{
	if(m_callbackInfos) {
		for(auto &cbInfo : *m_callbackInfos) {
			if(cbInfo.hCallback.IsValid() == false)
				continue;
			cbInfo.hCallback.Remove();
		}
		m_callbackInfos = nullptr;
	}
	GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().DeregisterComponent(*this);
}
void BaseEntityComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_ENTITY_COMPONENT_ADDED = registerEvent("ON_ENTITY_COMPONENT_ADDED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_ENTITY_COMPONENT_REMOVED = registerEvent("ON_ENTITY_COMPONENT_REMOVED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_MEMBERS_CHANGED = registerEvent("ON_MEMBERS_CHANGED", ComponentEventInfo::Type::Broadcast);
}
void BaseEntityComponent::Log(const std::string &msg, LogSeverity severity) const
{
	auto print = [&msg](auto &con) { con << msg << Con::endl; };
	switch(severity) {
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
	if(genericC) {
		CEOnMembersChanged ev {*this};
		genericC->InvokeEventCallbacks(BaseGenericComponent::EVENT_ON_MEMBERS_CHANGED, ev);
	}
}
void BaseEntityComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
static umath::ScaledTransform get_entity_pose(const pragma::BaseEntityComponent &component) { return component.GetEntity().GetPose(); }
template<typename TValue>
TValue get_identity_value()
{
	if constexpr(std::is_same_v<TValue, Quat>)
		return uquat::identity();
	return TValue {};
}
template<typename TValue, bool parentSpaceOnly = false>
static bool get_transform_member_value(const pragma::BaseEntityComponent &component, ComponentMemberIndex idx, umath::CoordinateSpace space, TValue &outValue)
{
	auto *memberInfo = component.GetMemberInfo(idx);
	if(!memberInfo || !memberInfo->getterFunction || (space != umath::CoordinateSpace::Local && space != umath::CoordinateSpace::World && space != umath::CoordinateSpace::Object))
		return false;
	if constexpr(std::is_same_v<TValue, umath::Transform> || std::is_same_v<TValue, umath::ScaledTransform>) {
		if(memberInfo->type != pragma::ents::EntityMemberType::Transform && memberInfo->type != pragma::ents::EntityMemberType::ScaledTransform) {
			auto *cPoseType = memberInfo->FindTypeMetaData<pragma::ents::PoseComponentTypeMetaData>();
			if(cPoseType) {
				// The requested type is Transform/ScaledTransform, but the property type is
				// position or rotation. In this case we want to take our pos/rot siblings into account, so
				// we return the pose for our associated pose instead.
				auto idxPose = cPoseType ? component.GetMemberIndex(cPoseType->poseProperty) : std::optional<ComponentMemberIndex> {};
				return idxPose.has_value() ? get_transform_member_value(component, *idxPose, space, outValue) : false;
			}
			return false;
		}
	}
	auto *cMetaData = memberInfo->FindTypeMetaData<pragma::ents::CoordinateTypeMetaData>();
	if constexpr(!parentSpaceOnly) {
		auto result = pragma::ents::visit_member(memberInfo->type, [&component, memberInfo, &outValue](auto tag) -> bool {
			using T = typename decltype(tag)::type;
			if constexpr(!udm::is_convertible<T, TValue>())
				return false;
			else {
				T value;
				memberInfo->getterFunction(*memberInfo, const_cast<pragma::BaseEntityComponent &>(component), &value);
				outValue = udm::convert<T, TValue>(value);
				return true;
			}
		});
		if(!result)
			return false;
	}
	auto idxParent = cMetaData ? component.GetMemberIndex(cMetaData->parentProperty) : std::optional<ComponentMemberIndex> {};
	if constexpr(parentSpaceOnly) {
		if(!idxParent.has_value()) {
			// If parentProperty is empty, then it's okay if no parent has been found, otherwise it's an error
			if(cMetaData->parentProperty.empty()) {
				outValue = get_identity_value<TValue>();
				return true;
			}
			return false;
		}
		switch(space) {
		case umath::CoordinateSpace::Local:
			{
				outValue = get_identity_value<TValue>();
				return true; // Nothing to do
			}
		default:
			return get_transform_member_value(component, *idxParent, space, outValue);
		}
		return false; // Unreachable
	}
	else {
		auto valueSpace = cMetaData ? cMetaData->space : umath::CoordinateSpace::World;
		if(valueSpace == space)
			return true;
		switch(valueSpace) {
		case umath::CoordinateSpace::Local:
			{
				umath::ScaledTransform parentPose {};
				if(!idxParent.has_value()) {
					if(space == umath::CoordinateSpace::Object)
						return true;
					parentPose = get_entity_pose(component);
				}
				else if(!get_transform_member_value<umath::ScaledTransform, false>(component, *idxParent, space, parentPose))
					return false;
				outValue = parentPose * outValue;
				return true;
			}
		case umath::CoordinateSpace::Object:
			{
				if(space == umath::CoordinateSpace::World) {
					// Effectively the same as the else branch below, but faster
					outValue = get_entity_pose(component) * outValue;
					return true;
				}
				else {
					if(!idxParent.has_value())
						return true; // Object space is equivalent to local space
					umath::ScaledTransform parentPose {};
					if(!get_transform_member_value<umath::ScaledTransform, false>(component, *idxParent, space, parentPose))
						return false;
					outValue = parentPose.GetInverse() * outValue;
					return true;
				}
				return false;
			}
		case umath::CoordinateSpace::World:
			{
				if(space == umath::CoordinateSpace::Object || !idxParent.has_value()) {
					// Effectively the same as the else branch below, but faster
					outValue = get_entity_pose(component).GetInverse() * outValue;
					return true;
				}
				else {
					umath::ScaledTransform parentPose {};
					if(!get_transform_member_value<umath::ScaledTransform, false>(component, *idxParent, space, parentPose))
						return false;
					outValue = parentPose.GetInverse() * outValue;
					return true;
				}
				return false;
			}
		}
		return false;
	}
	return false; // Unreachable
}
template<typename TValue>
static bool set_transform_member_value(pragma::BaseEntityComponent &component, ComponentMemberIndex idx, umath::CoordinateSpace space, const TValue &value)
{
	auto *memberInfo = component.GetMemberInfo(idx);
	if(!memberInfo || !memberInfo->setterFunction || (space != umath::CoordinateSpace::Local && space != umath::CoordinateSpace::World && space != umath::CoordinateSpace::Object))
		return false;
	auto *cMetaData = memberInfo->FindTypeMetaData<pragma::ents::CoordinateTypeMetaData>();
	auto setValue = [memberInfo, &component](const TValue &value) {
		return pragma::ents::visit_member(memberInfo->type, [&component, memberInfo, &value](auto tag) -> bool {
			using T = typename decltype(tag)::type;
			if constexpr(!udm::is_convertible<TValue, T>())
				return false;
			else {
				auto cnvVal = udm::convert<TValue, T>(value);
				memberInfo->setterFunction(*memberInfo, component, &cnvVal);
				return true;
			}
		});
	};
	auto valueSpace = cMetaData ? cMetaData->space : umath::CoordinateSpace::World;
	if(valueSpace == space)
		return setValue(value);
	auto idxParent = cMetaData ? component.GetMemberIndex(cMetaData->parentProperty) : std::optional<ComponentMemberIndex> {};
	switch(space) {
	case umath::CoordinateSpace::Local:
		{
			umath::ScaledTransform parentPose {};
			if(!idxParent.has_value()) {
				if(valueSpace == umath::CoordinateSpace::World)
					parentPose = get_entity_pose(component);
			}
			else if(!get_transform_member_value(component, *idxParent, valueSpace, parentPose))
				return false;
			auto valueInPropSpace = parentPose * value;
			return setValue(valueInPropSpace);
		}
	case umath::CoordinateSpace::Object:
		{
			if(valueSpace == umath::CoordinateSpace::World) {
				// Effectively the same as the else branch below, but faster
				auto valueInPropSpace = get_entity_pose(component) * value;
				return setValue(valueInPropSpace);
			}
			else {
				if(!idxParent.has_value())
					return setValue(value); // Object space is equivalent to local space
				umath::ScaledTransform parentPose {};
				if(!get_transform_member_value(component, *idxParent, valueSpace, parentPose))
					return false;
				auto valueInPropSpace = parentPose.GetInverse() * value;
				return setValue(valueInPropSpace);
			}
			return false;
		}
	case umath::CoordinateSpace::World:
		{
			if(valueSpace == umath::CoordinateSpace::Object || !idxParent.has_value()) {
				// Effectively the same as the else branch below, but faster
				auto valueInPropSpace = get_entity_pose(component).GetInverse() * value;
				return setValue(valueInPropSpace);
			}
			else {
				umath::ScaledTransform parentPose;
				if(!idxParent.has_value())
					parentPose = get_entity_pose(component);
				else if(!get_transform_member_value(component, *idxParent, umath::CoordinateSpace::World, parentPose))
					return false;
				auto valueInPropSpace = parentPose.GetInverse() * value;
				return setValue(valueInPropSpace);
			}
			return false;
		}
	}
	return false;
}
bool BaseEntityComponent::GetTransformMemberPos(ComponentMemberIndex idx, umath::CoordinateSpace space, Vector3 &outPos) const { return get_transform_member_value(*this, idx, space, outPos); }
bool BaseEntityComponent::GetTransformMemberRot(ComponentMemberIndex idx, umath::CoordinateSpace space, Quat &outRot) const { return get_transform_member_value(*this, idx, space, outRot); }
bool BaseEntityComponent::GetTransformMemberScale(ComponentMemberIndex idx, umath::CoordinateSpace space, Vector3 &outScale) const { return get_transform_member_value(*this, idx, space, outScale); }
bool BaseEntityComponent::GetTransformMemberPose(ComponentMemberIndex idx, umath::CoordinateSpace space, umath::ScaledTransform &outPose) const { return get_transform_member_value(*this, idx, space, outPose); }

bool BaseEntityComponent::SetTransformMemberPos(ComponentMemberIndex idx, umath::CoordinateSpace space, const Vector3 &pos) { return set_transform_member_value(*this, idx, space, pos); }
bool BaseEntityComponent::SetTransformMemberRot(ComponentMemberIndex idx, umath::CoordinateSpace space, const Quat &rot) { return set_transform_member_value(*this, idx, space, rot); }
bool BaseEntityComponent::SetTransformMemberScale(ComponentMemberIndex idx, umath::CoordinateSpace space, const Vector3 &scale) { return set_transform_member_value(*this, idx, space, scale); }
bool BaseEntityComponent::SetTransformMemberPose(ComponentMemberIndex idx, umath::CoordinateSpace space, const umath::ScaledTransform &pose) { return set_transform_member_value(*this, idx, space, pose); }

bool BaseEntityComponent::GetTransformMemberParentPos(ComponentMemberIndex idx, umath::CoordinateSpace space, Vector3 &outPos) const { return get_transform_member_value<std::remove_reference_t<decltype(outPos)>, true>(*this, idx, space, outPos); }
bool BaseEntityComponent::GetTransformMemberParentRot(ComponentMemberIndex idx, umath::CoordinateSpace space, Quat &outRot) const { return get_transform_member_value<std::remove_reference_t<decltype(outRot)>, true>(*this, idx, space, outRot); }
bool BaseEntityComponent::GetTransformMemberParentScale(ComponentMemberIndex idx, umath::CoordinateSpace space, Vector3 &outScale) const { return get_transform_member_value<std::remove_reference_t<decltype(outScale)>, true>(*this, idx, space, outScale); }
bool BaseEntityComponent::GetTransformMemberParentPose(ComponentMemberIndex idx, umath::CoordinateSpace space, umath::ScaledTransform &outPose) const { return get_transform_member_value<std::remove_reference_t<decltype(outPose)>, true>(*this, idx, space, outPose); }

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
const ComponentInfo *BaseEntityComponent::GetComponentInfo() const { return GetEntity().GetComponentManager()->GetComponentInfo(GetComponentId()); }
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
util::TWeakSharedHandle<const BaseEntityComponent> BaseEntityComponent::GetHandle() const { return GetHandle<BaseEntityComponent>(); }
util::TWeakSharedHandle<BaseEntityComponent> BaseEntityComponent::GetHandle() { return GetHandle<BaseEntityComponent>(); }
void BaseEntityComponent::Initialize()
{
	InitializeLuaObject(GetEntity().GetLuaState());

	auto &ent = GetEntity();
	OnAttached(ent);
	if(ent.IsSpawned()) {
		OnEntitySpawn();
		OnEntityPostSpawn();
	}
}
void BaseEntityComponent::OnRemove()
{
	OnDetached(GetEntity());
	if(m_eventCallbacks) {
		for(auto &pair : *m_eventCallbacks) {
			for(auto &hCb : pair.second) {
				if(hCb.IsValid() == false)
					continue;
				hCb.Remove();
			}
		}
		m_eventCallbacks = nullptr;
	}
	if(m_boundEvents) {
		for(auto &pair : *m_boundEvents) {
			for(auto &hCb : pair.second) {
				if(hCb.IsValid() == false)
					continue;
				hCb.Remove();
			}
		}
		m_boundEvents = nullptr;
	}
	if(umath::is_flag_set(m_stateFlags, StateFlags::IsLogicEnabled)) {
		auto &logicComponents = GetEntity().GetNetworkState()->GetGameState()->GetEntityTickComponents();
		*std::find(logicComponents.begin(), logicComponents.end(), this) = nullptr;
		umath::set_flag(m_stateFlags, StateFlags::IsLogicEnabled, false);
	}
}
bool BaseEntityComponent::ShouldTransmitNetData() const { return false; }
bool BaseEntityComponent::ShouldTransmitSnapshotData() const { return false; }
void BaseEntityComponent::FlagCallbackForRemoval(const CallbackHandle &hCallback, CallbackType cbType, BaseEntityComponent *component)
{
	switch(cbType) {
	case CallbackType::Entity:
		component = nullptr;
		break;
	case CallbackType::Component:
		if(component == nullptr)
			component = this;
		break;
	}
	auto &callbackInfos = GetCallbackInfos();
	callbackInfos.push_back({});
	auto &cbInfo = callbackInfos.back();
	cbInfo.hCallback = hCallback;
	cbInfo.pComponent = component;
}
const BaseEntity &BaseEntityComponent::GetEntity() const { return const_cast<BaseEntityComponent *>(this)->GetEntity(); }
BaseEntity &BaseEntityComponent::GetEntity() { return m_entity; }
const BaseEntity &BaseEntityComponent::operator->() const { return GetEntity(); }
BaseEntity &BaseEntityComponent::operator->() { return GetEntity(); }
ComponentId BaseEntityComponent::GetComponentId() const { return m_componentId; }
std::vector<BaseEntityComponent::CallbackInfo> &BaseEntityComponent::GetCallbackInfos() const
{
	if(!m_callbackInfos)
		m_callbackInfos = std::make_unique<std::vector<CallbackInfo>>();
	return *m_callbackInfos;
}
std::unordered_map<ComponentEventId, std::vector<CallbackHandle>> &BaseEntityComponent::GetEventCallbacks() const
{
	if(!m_eventCallbacks)
		m_eventCallbacks = std::make_unique<std::unordered_map<ComponentEventId, std::vector<CallbackHandle>>>();
	return *m_eventCallbacks;
}
std::unordered_map<ComponentEventId, std::vector<CallbackHandle>> &BaseEntityComponent::GetBoundEvents() const
{
	if(!m_boundEvents)
		m_boundEvents = std::make_unique<std::unordered_map<ComponentEventId, std::vector<CallbackHandle>>>();
	return *m_boundEvents;
}
CallbackHandle BaseEntityComponent::AddEventCallback(ComponentEventId eventId, const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback)
{
	return AddEventCallback(eventId, FunctionCallback<util::EventReply, std::reference_wrapper<ComponentEvent>>::Create(fCallback));
}
CallbackHandle BaseEntityComponent::AddEventCallback(ComponentEventId eventId, const CallbackHandle &hCallback)
{
	// Sanity check (to make sure the event type is actually associated with this component)
	auto componentTypeIndex = std::type_index(typeid(*this));
	auto baseTypeIndex = componentTypeIndex;
	GetBaseTypeIndex(baseTypeIndex);
	auto &events = GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
	auto it = events.find(eventId);
	if(it != events.end() && it->second.typeIndex.has_value() && componentTypeIndex != *it->second.typeIndex && baseTypeIndex != *it->second.typeIndex)
		throw std::logic_error("Attempted to add callback for component event " + std::to_string(eventId) + " (" + it->second.name + ") to component " + std::string(typeid(*this).name()) + ", which this event does not belong to!");

	auto &eventCallbacks = GetEventCallbacks();
	auto itEv = eventCallbacks.find(eventId);
	if(itEv == eventCallbacks.end())
		itEv = eventCallbacks.insert(std::make_pair(eventId, std::vector<CallbackHandle> {})).first;
	itEv->second.push_back(hCallback);
	return itEv->second.back();
}
void BaseEntityComponent::RemoveEventCallback(ComponentEventId eventId, const CallbackHandle &hCallback)
{
	if(!m_eventCallbacks)
		return;
	auto &eventCallbacks = GetEventCallbacks();
	auto itEv = eventCallbacks.find(eventId);
	if(itEv == eventCallbacks.end())
		return;
	auto itCb = std::find(itEv->second.begin(), itEv->second.end(), hCallback);
	if(itCb == itEv->second.end())
		return;
	itEv->second.erase(itCb);
	if(itEv->second.empty())
		eventCallbacks.erase(itEv);
}
util::EventReply BaseEntityComponent::InvokeEventCallbacks(ComponentEventId eventId, const ComponentEvent &evData) const
{
	return InvokeEventCallbacks(eventId, const_cast<ComponentEvent &>(evData)); // Hack: This assumes the argument was passed as temporary variable and changing it does not matter
}
util::EventReply BaseEntityComponent::InvokeEventCallbacks(ComponentEventId eventId, ComponentEvent &evData) const
{
	if(!m_eventCallbacks)
		return util::EventReply::Unhandled;
	auto &eventCallbacks = GetEventCallbacks();
	auto itEv = eventCallbacks.find(eventId);
	if(itEv == eventCallbacks.end())
		return util::EventReply::Unhandled;
	auto hThis = GetHandle();
	auto &evs = itEv->second;
	for(auto i = decltype(evs.size()) {0u}; i < evs.size();) {
		auto &hCb = evs[i];
		if(hCb.IsValid() == false) {
			itEv->second.erase(itEv->second.begin() + i);
			continue;
		}
		if(hCb.Call<util::EventReply, std::reference_wrapper<ComponentEvent>>(std::reference_wrapper<ComponentEvent>(evData)) == util::EventReply::Handled)
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
	return InvokeEventCallbacks(eventId, ev);
}
util::EventReply BaseEntityComponent::BroadcastEvent(ComponentEventId eventId, const ComponentEvent &evData) const
{
	return BroadcastEvent(eventId, const_cast<ComponentEvent &>(evData)); // Hack: This assumes the argument was passed as temporary variable and changing it does not matter
}
util::EventReply BaseEntityComponent::BroadcastEvent(ComponentEventId eventId, ComponentEvent &evData) const
{
	auto &ent = GetEntity();
	if(ent.BroadcastEvent(eventId, evData, this) == util::EventReply::Handled)
		return util::EventReply::Handled;
	return InvokeEventCallbacks(eventId, evData);
}
util::EventReply BaseEntityComponent::BroadcastEvent(ComponentEventId eventId) const
{
	CEGenericComponentEvent ev {};
	return BroadcastEvent(eventId, ev);
}
util::EventReply BaseEntityComponent::InjectEvent(ComponentEventId eventId, const ComponentEvent &evData) { return InjectEvent(eventId, const_cast<ComponentEvent &>(evData)); }
util::EventReply BaseEntityComponent::InjectEvent(ComponentEventId eventId, ComponentEvent &evData) { return HandleEvent(eventId, evData); }
util::EventReply BaseEntityComponent::InjectEvent(ComponentEventId eventId)
{
	CEGenericComponentEvent ev {};
	return BroadcastEvent(eventId, ev);
}
CallbackHandle BaseEntityComponent::BindEventUnhandled(ComponentEventId eventId, const std::function<void(std::reference_wrapper<ComponentEvent>)> &fCallback)
{
	return BindEvent(eventId, [fCallback](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		fCallback(evData);
		return util::EventReply::Unhandled;
	});
}
CallbackHandle BaseEntityComponent::BindEvent(ComponentEventId eventId, const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback)
{
	auto hCallback = FunctionCallback<util::EventReply, std::reference_wrapper<ComponentEvent>>::Create(fCallback);
	auto &ent = GetEntity();
	auto &events = ent.GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
	auto itInfo = events.find(eventId);
	if(itInfo != events.end()) {
		auto &info = itInfo->second;
		if(info.typeIndex.has_value()) {
			for(auto &pComponent : ent.GetComponents()) {
				if(pComponent.expired())
					continue;
				auto componentTypeIndex = std::type_index(typeid(*pComponent));
				auto baseTypeIndex = componentTypeIndex;
				pComponent->GetBaseTypeIndex(baseTypeIndex);
				if(componentTypeIndex != *info.typeIndex && baseTypeIndex != *info.typeIndex)
					continue;
				auto cb = pComponent->AddEventCallback(eventId, hCallback);
				FlagCallbackForRemoval(cb, CallbackType::Component, pComponent.get());
				return cb;
			}
		}
	}
	auto &boundEvents = GetBoundEvents();
	auto itEv = boundEvents.find(eventId);
	if(itEv == boundEvents.end())
		itEv = boundEvents.insert(std::make_pair(eventId, std::vector<CallbackHandle> {})).first;
	itEv->second.push_back(hCallback);
	return itEv->second.back();
}
util::EventReply BaseEntityComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(eventId == BaseEntity::EVENT_ON_SPAWN)
		OnEntitySpawn();
	else if(eventId == BaseEntity::EVENT_ON_POST_SPAWN)
		OnEntityPostSpawn();

	if(!m_boundEvents)
		return util::EventReply::Unhandled;
	auto &boundEvents = GetBoundEvents();
	auto itEv = boundEvents.find(eventId);
	if(itEv == boundEvents.end())
		return util::EventReply::Unhandled;
	for(auto it = itEv->second.begin(); it != itEv->second.end();) {
		auto &hCb = *it;
		if(hCb.IsValid() == false) {
			it = itEv->second.erase(it);
			continue;
		}
		if(hCb.Call<util::EventReply, std::reference_wrapper<ComponentEvent>>(std::reference_wrapper<ComponentEvent>(evData)) == util::EventReply::Handled)
			return util::EventReply::Handled;
		++it;
	}
	return util::EventReply::Unhandled;
}
void BaseEntityComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const {}
void BaseEntityComponent::OnEntityComponentAdded(BaseEntityComponent &component) {}
void BaseEntityComponent::OnEntityComponentAdded(BaseEntityComponent &component, bool bSkipEventBinding)
{
	if(bSkipEventBinding == false) {
		if(m_boundEvents) {
			auto &events = GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
			for(auto &pair : *m_boundEvents) {
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
					component.AddEventCallback(evId, hCb);
			}
		}
	}
	OnEntityComponentAdded(component);
}
void BaseEntityComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	if(m_boundEvents) {
		auto &events = GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEvents();
		for(auto &pair : *m_boundEvents) {
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
				component.RemoveEventCallback(evId, hCb);
		}
	}
	if(m_callbackInfos) {
		for(auto it = m_callbackInfos->begin(); it != m_callbackInfos->end();) {
			auto &cbInfo = *it;
			if(cbInfo.pComponent != &component && cbInfo.hCallback.IsValid()) {
				++it;
				continue;
			}
			if(cbInfo.hCallback.IsValid())
				it->hCallback.Remove();
			it = m_callbackInfos->erase(it);
		}
		if(m_callbackInfos->empty())
			m_callbackInfos = nullptr;
	}
	pragma::CEOnEntityComponentRemoved evData {*this};
	auto *genericC = GetEntity().GetGenericComponent();
	if(BroadcastEvent(EVENT_ON_ENTITY_COMPONENT_REMOVED, evData) != util::EventReply::Handled && genericC)
		genericC->InvokeEventCallbacks(BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED, evData);
}
Game &BaseEntityComponent::GetGame() { return *GetNetworkState().GetGameState(); }
NetworkState &BaseEntityComponent::GetNetworkState() { return *GetEntity().GetNetworkState(); }
EntityComponentManager &BaseEntityComponent::GetComponentManager() { return GetGame().GetEntityComponentManager(); }
void BaseEntityComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	udm["version"] = GetVersion();

	auto tCur = GetEntity().GetNetworkState()->GetGameState()->CurTime();
	udm["lastTick"] = m_tickData.lastTick - tCur;
	udm["nextTick"] = m_tickData.nextTick - tCur;
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
	Load(udm, version);
}
void BaseEntityComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) {}
void BaseEntityComponent::OnEntitySpawn() {}
void BaseEntityComponent::OnEntityPostSpawn() { UpdateTickPolicy(); }
void BaseEntityComponent::OnAttached(BaseEntity &ent) {}
void BaseEntityComponent::OnDetached(BaseEntity &ent)
{
	if(m_callbackInfos) {
		for(auto it = m_callbackInfos->begin(); it != m_callbackInfos->end();) {
			auto &cbInfo = *it;
			if(cbInfo.pComponent != nullptr && cbInfo.hCallback.IsValid()) {
				++it;
				continue;
			}
			if(cbInfo.hCallback.IsValid())
				it->hCallback.Remove();
			it = m_callbackInfos->erase(it);
		}
		if(m_callbackInfos->empty())
			m_callbackInfos = nullptr;
	}
}
pragma::NetEventId BaseEntityComponent::SetupNetEvent(const std::string &name) const { return GetEntity().GetNetworkState()->GetGameState()->SetupNetEvent(name); }

//////////////////

TickPolicy BaseEntityComponent::GetTickPolicy() const { return m_tickData.tickPolicy; }

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
	if(ShouldThink()) {
		if(umath::is_flag_set(m_stateFlags, StateFlags::IsLogicEnabled))
			return;
		logicComponents.push_back(this);
		umath::set_flag(m_stateFlags, StateFlags::IsLogicEnabled);
		return;
	}
	if(!umath::is_flag_set(m_stateFlags, StateFlags::IsLogicEnabled))
		return;
	logicComponents.erase(std::find(logicComponents.begin(), logicComponents.end(), this));
	umath::set_flag(m_stateFlags, StateFlags::IsLogicEnabled, false);
}
void BaseEntityComponent::SetTickPolicy(TickPolicy policy)
{
	if(policy == m_tickData.tickPolicy)
		return;
	m_tickData.tickPolicy = policy;

	if(umath::is_flag_set(m_stateFlags, StateFlags::IsThinking))
		return; // Tick policy update will be handled by game
	UpdateTickPolicy();
}

double BaseEntityComponent::GetNextTick() const { return m_tickData.nextTick; }
void BaseEntityComponent::SetNextTick(double t) { m_tickData.nextTick = t; }

double BaseEntityComponent::LastTick() const { return m_tickData.lastTick; }

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

	if(ShouldThink() == false) {
		m_stateFlags &= ~pragma::BaseEntityComponent::StateFlags::IsLogicEnabled;
		return false; // Game will handle removal from tick componentlist
	}
	return true;
}

std::string BaseEntityComponent::GetUri() const
{
	auto uri = GetUri(nullptr, GetEntity().GetUuid(), std::string {*GetComponentInfo()->name});
	assert(uri.has_value());
	return *uri;
}
std::string BaseEntityComponent::GetMemberUri(const std::string &memberName) const
{
	auto uri = GetUri();
	auto q = uri.find('?');
	return uri.substr(0, q) + "/" + memberName + uri.substr(q);
}
std::optional<std::string> BaseEntityComponent::GetMemberUri(ComponentMemberIndex memberIdx) const
{
	auto *info = GetMemberInfo(memberIdx);
	if(!info)
		return {};
	return GetMemberUri(GetEntity().GetNetworkState()->GetGameState(), GetEntity().GetUuid(), GetComponentId(), *info->GetName());
}
std::optional<std::string> BaseEntityComponent::GetUri(Game *game, std::variant<util::Uuid, std::string> entityIdentifier, std::variant<ComponentId, std::string> componentIdentifier)
{
	return std::visit(
	  [&componentIdentifier, game](auto &value) -> std::optional<std::string> {
		  auto uri = BaseEntity::GetUri(value);
		  auto q = uri.find('?');
		  auto componentName = std::visit(
		    [game](auto &value) -> std::optional<std::string> {
			    using T = util::base_type<decltype(value)>;
			    if constexpr(std::is_same_v<T, ComponentId>) {
				    if(!game)
					    return {};
				    auto *info = game->GetEntityComponentManager().GetComponentInfo(value);
				    if(!info)
					    return {};
				    return std::string {*info->name};
			    }
			    else
				    return value;
		    },
		    componentIdentifier);
		  if(!componentName.has_value())
			  return {};
		  return uri.substr(0, q) + "/ec/" + *componentName + uri.substr(q);
	  },
	  entityIdentifier);
}
std::optional<std::string> BaseEntityComponent::GetMemberUri(Game *game, std::variant<util::Uuid, std::string> entityIdentifier, std::variant<ComponentId, std::string> componentIdentifier, std::variant<ComponentMemberIndex, std::string> memberIdentifier)
{
	auto uri = GetUri(game, entityIdentifier, componentIdentifier);
	if(!uri.has_value())
		return {};
	auto memberName = std::visit(
	  [game, &componentIdentifier](auto &memberId) -> std::optional<std::string> {
		  using T = util::base_type<decltype(memberId)>;
		  if constexpr(std::is_same_v<T, ComponentMemberIndex>) {
			  if(!game)
				  return {};
			  return std::visit(
			    [game, &memberId](auto &value) -> std::optional<std::string> {
				    using T = util::base_type<decltype(value)>;
				    ComponentId componentId;
				    if constexpr(std::is_same_v<T, ComponentId>)
					    componentId = value;
				    else {
					    if(game->GetEntityComponentManager().GetComponentTypeId(value, componentId) == false)
						    return {};
				    }

				    auto *info = game->GetEntityComponentManager().GetComponentInfo(componentId);
				    if(!info || memberId >= info->members.size())
					    return {};
				    auto &memberInfo = info->members[memberId];
				    return std::string {*memberInfo.GetName()};
			    },
			    componentIdentifier);
		  }
		  else
			  return memberId;
	  },
	  memberIdentifier);
	if(!memberName.has_value())
		return {};
	auto q = uri->find('?');
	return uri->substr(0, q) + "/" + *memberName + uri->substr(q);
}
