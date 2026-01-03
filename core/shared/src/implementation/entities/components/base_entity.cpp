// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include <cassert>

module pragma.shared;

import :entities.components.base;

import panima;

using namespace pragma;

DLLNETWORK std::ostream &pragma::operator<<(std::ostream &os, const BaseEntityComponent &component)
{
	auto *info = component.GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(component.GetComponentId());
	os << "Component[";
	if(!info)
		os << "NULL";
	else
		os << info->name << "][" << info->id << "][" << magic_enum::enum_flags_name(info->flags);
	os << "]";
	return os;
}

decltype(EEntityComponentCallbackEvent::Count) EEntityComponentCallbackEvent::Count = EEntityComponentCallbackEvent {math::to_integral(E::Count)};
BaseEntityComponent::BaseEntityComponent(ecs::BaseEntity &ent) : m_entity {ent} {}
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
	if(!math::is_flag_set(m_stateFlags, StateFlags::CleanedUp)) {
		auto *info = this->GetComponentInfo();
		std::string typeName = info ? info->name : "UNKNOWN";
		std::string msg = "Component of type '" + typeName + "' was not cleaned up properly! Was :CleanUp not called?";
		Con::CERR << msg << Con::endl;
		throw std::runtime_error {msg};
	}
	GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().DeregisterComponent(*this);
}
void BaseEntityComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED = registerEvent("ON_ENTITY_COMPONENT_ADDED", ComponentEventInfo::Type::Broadcast);
	baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED = registerEvent("ON_ENTITY_COMPONENT_REMOVED", ComponentEventInfo::Type::Broadcast);
	baseEntityComponent::EVENT_ON_MEMBERS_CHANGED = registerEvent("ON_MEMBERS_CHANGED", ComponentEventInfo::Type::Broadcast);
	baseEntityComponent::EVENT_ON_ACTIVE_STATE_CHANGED = registerEvent("ON_ACTIVE_STATE_CHANGED", ComponentEventInfo::Type::Broadcast);
}

spdlog::logger &BaseEntityComponent::InitLogger() const
{
	auto *info = GetComponentInfo();
	assert(info != nullptr);
	return register_logger("c_" + std::string {info->name.str});
}

void BaseEntityComponent::Log(const std::string &msg, LogSeverity severity) const
{
	auto &logger = InitLogger();
	switch(severity) {
	case LogSeverity::Normal:
		logger.info(msg);
		break;
	case LogSeverity::Debug:
		logger.debug(msg);
		break;
	case LogSeverity::Warning:
		logger.warn(msg);
		break;
	case LogSeverity::Error:
		logger.error(msg);
		break;
	case LogSeverity::Critical:
		logger.critical(msg);
		break;
	}
}

bool BaseEntityComponent::IsPropertyAnimated(const std::string &property)
{
	auto &ent = GetEntity();
	auto panimaC = ent.GetComponent<PanimaComponent>();
	auto memberIdx = GetMemberIndex(property);
	auto *info = GetComponentInfo();
	if(panimaC.expired() || !info || !memberIdx)
		return false;
	std::string componentName {info->name.str};
	std::string targetPath = "ec/" + componentName + "/" + property;
	for(auto &amData : panimaC->GetAnimationManagers()) {
		auto &manager = amData->animationManager;
		auto &player = manager->GetPlayer();
		auto *anim = player.GetAnimation();
		if(!anim)
			continue;
		if(anim->FindChannel(targetPath))
			return true;
	}
	return false;
}
void BaseEntityComponent::SetPropertyAnimated(const std::string &property, bool animated)
{
	auto &ent = GetEntity();
	auto panimaC = ent.AddComponent<PanimaComponent>();
	auto memberIdx = GetMemberIndex(property);
	auto *info = GetComponentInfo();
	if(panimaC.expired() || !info || !memberIdx)
		return;
	if(!animated && !panimaC->GetAnimationManager("base"))
		return;
	std::string componentName {info->name.str};
	std::string targetPath = "ec/" + componentName + "/" + property;
	// We want the base manager to run before all others (so that it can be overwritten)
	constexpr int32_t priority = -10'000;
	auto baseManager = panimaC->AddAnimationManager("base", priority);
	if(!animated) {
		auto &player = baseManager->GetPlayer();
		auto *anim = const_cast<panima::Animation *>(player.GetAnimation());
		if(anim)
			anim->RemoveChannel(targetPath);
		return;
	}

	auto hasChannel = false;
	for(auto &amData : panimaC->GetAnimationManagers()) {
		auto &manager = amData->animationManager;
		auto &player = manager->GetPlayer();
		auto *anim = const_cast<panima::Animation *>(player.GetAnimation());
		if(!anim)
			continue;
		auto *channel = anim->FindChannel(targetPath);
		if(channel) {
			hasChannel = true;
			break;
		}
	}
	if(hasChannel == false) {
		auto &player = baseManager->GetPlayer();
		if(!player.GetAnimation()) {
			auto anim = pragma::util::make_shared<panima::Animation>();
			panimaC->PlayAnimation(*baseManager, *anim);
		}
		auto *anim = const_cast<panima::Animation *>(player.GetAnimation());
		if(anim) {
			auto *memberInfo = GetMemberInfo(*memberIdx);
			if(memberInfo && is_animatable_type(memberInfo->type)) {
				udm::visit_ng(static_cast<udm::Type>(memberInfo->type), [this, anim, &panimaC, &baseManager, &targetPath, memberInfo](auto tag) {
					using T = typename decltype(tag)::type;
					if constexpr(pragma::is_animatable_type_v<T>) {
						T value;
						memberInfo->getterFunction(*memberInfo, *this, &value);

						auto channel = anim->AddChannel(targetPath, udm::type_to_enum<T>());
						channel->template AddValue<T>(0.f, value);
						panimaC->PlayAnimation(*baseManager, *anim);
					}
				});
			}
		}
	}
}
void BaseEntityComponent::OnMembersChanged()
{
	BroadcastEvent(baseEntityComponent::EVENT_ON_MEMBERS_CHANGED);

	auto *genericC = GetEntity().GetGenericComponent();
	if(genericC) {
		CEOnMembersChanged ev {*this};
		genericC->InvokeEventCallbacks(baseGenericComponent::EVENT_ON_MEMBERS_CHANGED, ev);
	}
}
void BaseEntityComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
void BaseEntityComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts) {}
static math::ScaledTransform get_entity_pose(const BaseEntityComponent &component) { return component.GetEntity().GetPose(); }
template<typename TValue>
TValue get_identity_value()
{
	if constexpr(std::is_same_v<TValue, Quat>)
		return uquat::identity();
	return TValue {};
}
template<typename TValue, bool parentSpaceOnly = false>
static bool get_transform_member_value(const BaseEntityComponent &component, ComponentMemberIndex idx, math::CoordinateSpace space, TValue &outValue);
template<typename TValue, bool parentSpaceOnly = false>
static bool get_transform_member_value(const BaseEntityComponent &component, ComponentMemberIndex idx, math::CoordinateSpace space, TValue &outValue, void (*getValue)(const ComponentMemberInfo &, BaseEntityComponent &, void *, const void *))
{
	auto *memberInfo = component.GetMemberInfo(idx);
	if(!memberInfo || !memberInfo->getterFunction || (space != math::CoordinateSpace::Local && space != math::CoordinateSpace::World && space != math::CoordinateSpace::Object))
		return false;
	if constexpr(std::is_same_v<TValue, math::Transform> || std::is_same_v<TValue, math::ScaledTransform>) {
		if(memberInfo->type != ents::EntityMemberType::Transform && memberInfo->type != ents::EntityMemberType::ScaledTransform) {
			auto *cPoseType = memberInfo->FindTypeMetaData<ents::PoseComponentTypeMetaData>();
			if(cPoseType) {
				// The requested type is Transform/ScaledTransform, but the property type is
				// position or rotation. In this case we want to take our pos/rot siblings into account, so
				// we return the pose for our associated pose instead.
				auto idxPose = cPoseType ? component.GetMemberIndex(cPoseType->poseProperty) : std::optional<ComponentMemberIndex> {};
				return idxPose.has_value() ? get_transform_member_value(component, *idxPose, space, outValue, getValue) : false;
			}
			switch(memberInfo->type) {
			case ents::EntityMemberType::Vector3:
				{
					Vector3 value;
					if(!get_transform_member_value<Vector3, parentSpaceOnly>(component, idx, space, value, getValue))
						return false;
					outValue = {};
					outValue.SetOrigin(value);
					return true;
				}
			case ents::EntityMemberType::EulerAngles:
			case ents::EntityMemberType::Quaternion:
				{
					Quat value;
					if(!get_transform_member_value<Quat, parentSpaceOnly>(component, idx, space, value, getValue))
						return false;
					outValue = {};
					outValue.SetRotation(value);
					return true;
				}
			}
			return false;
		}
	}
	auto *cMetaData = memberInfo->FindTypeMetaData<ents::CoordinateTypeMetaData>();
	if constexpr(!parentSpaceOnly) {
		auto result = ents::visit_member(memberInfo->type, [&component, memberInfo, &outValue, getValue](auto tag) -> bool {
			using T = typename decltype(tag)::type;
			if constexpr(!udm::is_convertible<T, TValue>())
				return false;
			else {
				T value;
				getValue(*memberInfo, const_cast<BaseEntityComponent &>(component), &value, &outValue);
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
		case math::CoordinateSpace::Local:
			{
				outValue = get_identity_value<TValue>();
				return true; // Nothing to do
			}
		default:
			return get_transform_member_value(component, *idxParent, space, outValue, getValue);
		}
		return false; // Unreachable
	}
	else {
		auto valueSpace = cMetaData ? cMetaData->space : math::CoordinateSpace::World;
		if(valueSpace == space)
			return true;
		switch(valueSpace) {
		case math::CoordinateSpace::Local:
			{
				math::ScaledTransform parentPose {};
				if(!idxParent.has_value()) {
					if(space == math::CoordinateSpace::Object)
						return true;
					parentPose = get_entity_pose(component);
				}
				else if(!get_transform_member_value<math::ScaledTransform, false>(component, *idxParent, space, parentPose))
					return false;
				outValue = parentPose * outValue;
				return true;
			}
		case math::CoordinateSpace::Object:
			{
				if(space == math::CoordinateSpace::World) {
					// Effectively the same as the else branch below, but faster
					outValue = get_entity_pose(component) * outValue;
					return true;
				}
				else {
					if(!idxParent.has_value())
						return true; // Object space is equivalent to local space
					math::ScaledTransform parentPose {};
					if(!get_transform_member_value<math::ScaledTransform, false>(component, *idxParent, space, parentPose))
						return false;
					outValue = parentPose.GetInverse() * outValue;
					return true;
				}
				return false;
			}
		case math::CoordinateSpace::World:
			{
				if(space == math::CoordinateSpace::Object || !idxParent.has_value()) {
					// Effectively the same as the else branch below, but faster
					outValue = get_entity_pose(component).GetInverse() * outValue;
					return true;
				}
				else {
					math::ScaledTransform parentPose {};
					if(!get_transform_member_value<math::ScaledTransform, false>(component, *idxParent, space, parentPose))
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
template<typename TValue, bool parentSpaceOnly>
bool get_transform_member_value(const BaseEntityComponent &component, ComponentMemberIndex idx, math::CoordinateSpace space, TValue &outValue)
{
	return get_transform_member_value<TValue, parentSpaceOnly>(component, idx, space, outValue, +[](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *outData, const void *) { memberInfo.getterFunction(memberInfo, component, outData); });
}

template<typename TValue>
static std::optional<TValue> get_value_in_property_space(BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, math::CoordinateSpace space, const TValue &value, math::CoordinateSpace *optOutPropSpace = nullptr)
{
	if((space != math::CoordinateSpace::Local && space != math::CoordinateSpace::World && space != math::CoordinateSpace::Object))
		return {};
	auto *cMetaData = memberInfo.FindTypeMetaData<ents::CoordinateTypeMetaData>();
	auto valueSpace = cMetaData ? cMetaData->space : math::CoordinateSpace::World;
	if(optOutPropSpace)
		*optOutPropSpace = valueSpace;
	if(valueSpace == space)
		return value;
	auto idxParent = cMetaData ? component.GetMemberIndex(cMetaData->parentProperty) : std::optional<ComponentMemberIndex> {};
	switch(space) {
	case math::CoordinateSpace::Local:
		{
			math::ScaledTransform parentPose {};
			if(!idxParent.has_value()) {
				if(valueSpace == math::CoordinateSpace::World)
					parentPose = get_entity_pose(component);
			}
			else if(!get_transform_member_value(component, *idxParent, valueSpace, parentPose))
				return {};
			auto valueInPropSpace = parentPose * value;
			return valueInPropSpace;
		}
	case math::CoordinateSpace::Object:
		{
			if(valueSpace == math::CoordinateSpace::World) {
				// Effectively the same as the else branch below, but faster
				auto valueInPropSpace = get_entity_pose(component) * value;
				return valueInPropSpace;
			}
			else {
				if(!idxParent.has_value())
					return value; // Object space is equivalent to local space
				math::ScaledTransform parentPose {};
				if(!get_transform_member_value(component, *idxParent, valueSpace, parentPose))
					return {};
				auto valueInPropSpace = parentPose.GetInverse() * value;
				return valueInPropSpace;
			}
			return {};
		}
	case math::CoordinateSpace::World:
		{
			if(valueSpace == math::CoordinateSpace::Object || !idxParent.has_value()) {
				// Effectively the same as the else branch below, but faster
				auto valueInPropSpace = get_entity_pose(component).GetInverse() * value;
				return valueInPropSpace;
			}
			else {
				math::ScaledTransform parentPose;
				if(!idxParent.has_value())
					parentPose = get_entity_pose(component);
				else if(!get_transform_member_value(component, *idxParent, math::CoordinateSpace::World, parentPose))
					return {};
				auto valueInPropSpace = parentPose.GetInverse() * value;
				return valueInPropSpace;
			}
			return {};
		}
	}
	return {};
}

template<typename TValue>
static bool set_transform_member_value(BaseEntityComponent &component, ComponentMemberIndex idx, math::CoordinateSpace space, const TValue &value)
{
	auto *memberInfo = component.GetMemberInfo(idx);
	if(!memberInfo)
		return {};
	auto setValue = [memberInfo, &component](const TValue &value) {
		return ents::visit_member(memberInfo->type, [&component, memberInfo, &value](auto tag) -> bool {
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
	auto valueInPropSpace = get_value_in_property_space<TValue>(component, *memberInfo, space, value);
	if(!valueInPropSpace)
		return false;
	return setValue(*valueInPropSpace);
}

template<class T>
bool convert_transform_member_value_to_target_space(const BaseEntityComponent &component, ComponentMemberIndex idx, math::CoordinateSpace space, T &inOutPos)
{
	return get_transform_member_value<T>(
	  component, idx, space, inOutPos, +[](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *outData, const void *inOutPos) {
		  if(!pragma::ents::is_udm_member_type(memberInfo.type))
			  throw std::invalid_argument {"Property has type '" + std::string {magic_enum::enum_name(memberInfo.type)} + "', which is not a valid UDM type!"};
		  constexpr auto type = udm::type_to_enum<T>();
		  auto memberType = static_cast<udm::Type>(memberInfo.type);
		  if(memberType == type)
			  memcpy(outData, inOutPos, sizeof(T));
		  else if(udm::is_convertible(memberType, type)) {
			  ents::visit_member(memberInfo.type, [inOutPos, outData](auto tag) {
				  using TMember = typename decltype(tag)::type;
				  if constexpr(!udm::is_convertible<TMember, T>()) {
					  // unreachable
					  return;
				  }
				  else {
					  auto &tmp = *static_cast<const TMember *>(inOutPos);
					  auto convValue = udm::convert<TMember, T>(tmp);
					  memcpy(outData, &convValue, sizeof(T));
					  return;
				  }
			  });
		  }
		  else
			  throw std::invalid_argument {"Property type '" + std::string {magic_enum::enum_name(memberInfo.type)} + "' is not compatible with expected type '" + std::string {magic_enum::enum_name(type)} + "'!"};
	  });
}
bool BaseEntityComponent::ConvertTransformMemberPosToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutPos) const { return convert_transform_member_value_to_target_space(*this, idx, space, inOutPos); }
bool BaseEntityComponent::ConvertTransformMemberRotToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &inOutRot) const { return convert_transform_member_value_to_target_space(*this, idx, space, inOutRot); }
bool BaseEntityComponent::ConvertTransformMemberScaleToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutScale) const { return convert_transform_member_value_to_target_space(*this, idx, space, inOutScale); }
bool BaseEntityComponent::ConvertTransformMemberPoseToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &inOutPose) const { return convert_transform_member_value_to_target_space(*this, idx, space, inOutPose); }

bool BaseEntityComponent::GetTransformMemberPos(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outPos) const { return get_transform_member_value(*this, idx, space, outPos); }
bool BaseEntityComponent::GetTransformMemberRot(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &outRot) const { return get_transform_member_value(*this, idx, space, outRot); }
bool BaseEntityComponent::GetTransformMemberScale(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outScale) const { return get_transform_member_value(*this, idx, space, outScale); }
bool BaseEntityComponent::GetTransformMemberPose(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &outPose) const { return get_transform_member_value(*this, idx, space, outPose); }

bool BaseEntityComponent::SetTransformMemberPos(ComponentMemberIndex idx, math::CoordinateSpace space, const Vector3 &pos) { return set_transform_member_value(*this, idx, space, pos); }
bool BaseEntityComponent::SetTransformMemberRot(ComponentMemberIndex idx, math::CoordinateSpace space, const Quat &rot) { return set_transform_member_value(*this, idx, space, rot); }
bool BaseEntityComponent::SetTransformMemberScale(ComponentMemberIndex idx, math::CoordinateSpace space, const Vector3 &scale) { return set_transform_member_value(*this, idx, space, scale); }
bool BaseEntityComponent::SetTransformMemberPose(ComponentMemberIndex idx, math::CoordinateSpace space, const math::ScaledTransform &pose) { return set_transform_member_value(*this, idx, space, pose); }

bool BaseEntityComponent::GetTransformMemberParentPos(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outPos) const { return get_transform_member_value<std::remove_reference_t<decltype(outPos)>, true>(*this, idx, space, outPos); }
bool BaseEntityComponent::GetTransformMemberParentRot(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &outRot) const { return get_transform_member_value<std::remove_reference_t<decltype(outRot)>, true>(*this, idx, space, outRot); }
bool BaseEntityComponent::GetTransformMemberParentScale(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outScale) const { return get_transform_member_value<std::remove_reference_t<decltype(outScale)>, true>(*this, idx, space, outScale); }
bool BaseEntityComponent::GetTransformMemberParentPose(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &outPose) const { return get_transform_member_value<std::remove_reference_t<decltype(outPose)>, true>(*this, idx, space, outPose); }

template<typename TValue>
static bool convert_value_to_member_space(const BaseEntityComponent &component, ComponentMemberIndex idx, math::CoordinateSpace space, TValue &inOutValue, math::CoordinateSpace *optOutMemberSpace)
{
	auto *memberInfo = component.GetMemberInfo(idx);
	if(!memberInfo)
		return {};
	auto valueInPropSpace = get_value_in_property_space<TValue>(const_cast<BaseEntityComponent &>(component), *memberInfo, space, inOutValue);
	if(!valueInPropSpace)
		return false;
	inOutValue = *valueInPropSpace;
	return true;
}
bool BaseEntityComponent::ConvertPosToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutPos, math::CoordinateSpace *optOutMemberSpace) const { return convert_value_to_member_space(*this, idx, space, inOutPos, optOutMemberSpace); }
bool BaseEntityComponent::ConvertRotToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &inOutRot, math::CoordinateSpace *optOutMemberSpace) const { return convert_value_to_member_space(*this, idx, space, inOutRot, optOutMemberSpace); }
bool BaseEntityComponent::ConvertScaleToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutScale, math::CoordinateSpace *optOutMemberSpace) const { return convert_value_to_member_space(*this, idx, space, inOutScale, optOutMemberSpace); }
bool BaseEntityComponent::ConvertPoseToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &inOutPose, math::CoordinateSpace *optOutMemberSpace) const { return convert_value_to_member_space(*this, idx, space, inOutPose, optOutMemberSpace); }

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
	string::to_lower(lname);
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
void BaseEntityComponent::Initialize() { InitializeLuaObject(GetEntity().GetLuaState()); }
void BaseEntityComponent::PostInitialize()
{
	auto &ent = GetEntity();
	OnAttached(ent);
	if(ent.IsSpawned()) {
		OnEntitySpawn();
		OnEntityPostSpawn();
	}
}
void BaseEntityComponent::CleanUp()
{
	math::set_flag(m_stateFlags, StateFlags::CleanedUp);
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
	if(math::is_flag_set(m_stateFlags, StateFlags::IsLogicEnabled)) {
		auto &logicComponents = GetEntity().GetNetworkState()->GetGameState()->GetEntityTickComponents();
		*std::find(logicComponents.begin(), logicComponents.end(), this) = nullptr;
		math::set_flag(m_stateFlags, StateFlags::IsLogicEnabled, false);
	}
}
void BaseEntityComponent::OnRemove() {}
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
const ecs::BaseEntity &BaseEntityComponent::GetEntity() const { return const_cast<BaseEntityComponent *>(this)->GetEntity(); }
ecs::BaseEntity &BaseEntityComponent::GetEntity() { return m_entity; }
const ecs::BaseEntity &BaseEntityComponent::operator->() const { return GetEntity(); }
ecs::BaseEntity &BaseEntityComponent::operator->() { return GetEntity(); }
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
	if(eventId == ecs::baseEntity::EVENT_ON_SPAWN)
		OnEntitySpawn();
	else if(eventId == ecs::baseEntity::EVENT_ON_POST_SPAWN)
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
	CEOnEntityComponentRemoved evData {*this};
	auto *genericC = GetEntity().GetGenericComponent();
	if(BroadcastEvent(baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED, evData) != util::EventReply::Handled && genericC)
		genericC->InvokeEventCallbacks(baseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED, evData);
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
void BaseEntityComponent::OnAttached(ecs::BaseEntity &ent) {}
void BaseEntityComponent::OnDetached(ecs::BaseEntity &ent)
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
NetEventId BaseEntityComponent::SetupNetEvent(const std::string &name) const { return GetEntity().GetNetworkState()->GetGameState()->SetupNetEvent(name); }

//////////////////

TickPolicy BaseEntityComponent::GetTickPolicy() const { return m_tickData.tickPolicy; }

bool BaseEntityComponent::ShouldThink() const
{
	if(!IsActive() || (m_tickData.tickPolicy != TickPolicy::Always && m_tickData.tickPolicy != TickPolicy::WhenVisible))
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
		if(math::is_flag_set(m_stateFlags, StateFlags::IsLogicEnabled))
			return;
		logicComponents.push_back(this);
		math::set_flag(m_stateFlags, StateFlags::IsLogicEnabled);
		return;
	}
	if(!math::is_flag_set(m_stateFlags, StateFlags::IsLogicEnabled))
		return;
	logicComponents.erase(std::find(logicComponents.begin(), logicComponents.end(), this));
	math::set_flag(m_stateFlags, StateFlags::IsLogicEnabled, false);
}
void BaseEntityComponent::SetTickPolicy(TickPolicy policy)
{
	if(policy == m_tickData.tickPolicy)
		return;
	m_tickData.tickPolicy = policy;

	if(math::is_flag_set(m_stateFlags, StateFlags::IsThinking))
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
	m_stateFlags |= StateFlags::IsThinking;

	auto hThis = GetHandle();
	auto &ent = GetEntity();
	OnTick(tDelta);
	if(hThis.expired())
		return true; // This component isn't valid anymore; Return immediately
	Game *game = ent.GetNetworkState()->GetGameState();
	m_tickData.lastTick = game->CurTime();

	m_stateFlags &= ~StateFlags::IsThinking;

	if(ShouldThink() == false) {
		m_stateFlags &= ~StateFlags::IsLogicEnabled;
		return false; // Game will handle removal from tick componentlist
	}
	return true;
}

void BaseEntityComponent::SetActive(bool enabled)
{
	if(enabled == IsActive())
		return;
	math::set_flag(m_stateFlags, StateFlags::IsInactive, !enabled);
	BroadcastEvent(baseEntityComponent::EVENT_ON_ACTIVE_STATE_CHANGED);
	OnActiveStateChanged(enabled);
	UpdateTickPolicy();
}
bool BaseEntityComponent::IsActive() const { return !math::is_flag_set(m_stateFlags, StateFlags::IsInactive); }
void BaseEntityComponent::Activate() { SetActive(true); }
void BaseEntityComponent::Deactivate() { SetActive(false); }
void BaseEntityComponent::OnActiveStateChanged(bool active) {}

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
		  auto uri = ecs::BaseEntity::GetUri(value);
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

spdlog::logger *pragma::find_logger(Game &game, std::type_index typeIndex)
{
	auto &componentManager = game.GetEntityComponentManager();
	ComponentId componentId;
	if(componentManager.GetComponentId(typeIndex, componentId)) {
		auto *info = componentManager.GetComponentInfo(componentId);
		if(info)
			return &register_logger("c_" + std::string {info->name.str});
	}
	return nullptr;
}

spdlog::logger &BaseEntityComponent::get_logger(std::type_index typeIndex)
{
	auto *engine = get_engine();

	for(auto *state : {Engine::Get()->GetClientState(), Engine::Get()->GetServerNetworkState()}) {
		if(!state)
			continue;
		auto *game = state->GetGameState();
		if(!game)
			continue;
		auto *logger = find_logger(*game, typeIndex);
		if(logger)
			return *logger;
	}
	return register_logger("c_unknown");
}
