/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/animated_2_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/model/model.h"
#include "pragma/model/animation/animation_manager.hpp"
#include "pragma/model/animation/animation.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/lua_call.hpp"
#include <panima/channel.hpp>
#include <panima/animation.hpp>
#include <panima/player.hpp>
#include <panima/channel_t.hpp>

using namespace pragma;
#pragma optimize("",off)
ComponentEventId Animated2Component::EVENT_HANDLE_ANIMATION_EVENT = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_ON_PLAY_ANIMATION = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_ON_ANIMATION_COMPLETE = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_ON_ANIMATION_START = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_MAINTAIN_ANIMATIONS = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_ON_ANIMATIONS_UPDATED = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_PLAY_ANIMATION = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_TRANSLATE_ANIMATION = pragma::INVALID_COMPONENT_ID;
ComponentEventId Animated2Component::EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER = pragma::INVALID_COMPONENT_ID;
void Animated2Component::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_HANDLE_ANIMATION_EVENT = componentManager.RegisterEvent("A2_HANDLE_ANIMATION_EVENT");
	EVENT_ON_PLAY_ANIMATION = componentManager.RegisterEvent("A2_ON_PLAY_ANIMATION");
	EVENT_ON_ANIMATION_COMPLETE = componentManager.RegisterEvent("A2_ON_ANIMATION_COMPLETE");
	EVENT_ON_ANIMATION_START = componentManager.RegisterEvent("A2_ON_ANIMATION_START");
	EVENT_MAINTAIN_ANIMATIONS = componentManager.RegisterEvent("A2_MAINTAIN_ANIMATIONS");
	EVENT_ON_ANIMATIONS_UPDATED = componentManager.RegisterEvent("A2_ON_ANIMATIONS_UPDATED");
	EVENT_PLAY_ANIMATION = componentManager.RegisterEvent("A2_PLAY_ANIMATION");
	EVENT_TRANSLATE_ANIMATION = componentManager.RegisterEvent("A2_TRANSLATE_ANIMATION");
	EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER = componentManager.RegisterEvent("A2_INITIALIZE_CHANNEL_VALUE_SUBMITTER");
}
Animated2Component::Animated2Component(BaseEntity &ent)
	: BaseEntityComponent(ent),m_playbackRate(util::FloatProperty::Create(1.f))
{}
void Animated2Component::SetPlaybackRate(float rate) {*m_playbackRate = rate;}
float Animated2Component::GetPlaybackRate() const {return *m_playbackRate;}
const util::PFloatProperty &Animated2Component::GetPlaybackRateProperty() const {return m_playbackRate;}
animation::PAnimationManager Animated2Component::AddAnimationManager()
{
	auto mdl = GetEntity().GetModel();
	if(!mdl)
		return nullptr;
	auto player = animation::AnimationManager::Create(*mdl);

	animation::AnimationPlayerCallbackInterface callbackInteface {};
	callbackInteface.onPlayAnimation = [this](panima::AnimationId animId,FPlayAnim flags) -> bool {
		CEAnim2OnPlayAnimation evData{animId,flags};
		return InvokeEventCallbacks(EVENT_PLAY_ANIMATION,evData) != util::EventReply::Handled;
	};
	callbackInteface.onStopAnimation = []() {
	
	};
	callbackInteface.translateAnimation = [this](panima::AnimationId &animId,FPlayAnim &flags) {
		CEAnim2TranslateAnimation evTranslateAnimData {animId,flags};
		InvokeEventCallbacks(EVENT_TRANSLATE_ANIMATION,evTranslateAnimData);
	};

	m_animationManagers.push_back(player);
	return player;
}
void Animated2Component::RemoveAnimationManager(const animation::AnimationManager &player)
{
	auto it = std::find_if(m_animationManagers.begin(),m_animationManagers.end(),[&player](const animation::PAnimationManager &playerOther) {
		return playerOther.get() == &player;
	});
	if(it == m_animationManagers.end())
		return;
	m_animationManagers.erase(it);
}
static constexpr uint8_t get_component_count(udm::Type type)
{
	if(udm::is_numeric_type(type))
		return 1;
	switch(type)
	{
	case udm::Type::Vector2:
	case udm::Type::Vector2i:
		return 2;
	case udm::Type::Vector3:
	case udm::Type::Vector3i:
	case udm::Type::EulerAngles:
		return 3;
	case udm::Type::Vector4:
	case udm::Type::Vector4i:
		return 4;
	}
	return 0;
}
static constexpr bool is_type_compatible(udm::Type channelType,udm::Type memberType)
{
	return get_component_count(channelType) <= get_component_count(memberType);
}

template<typename TChannel,typename TMember,auto TMapArray> requires(is_animatable_type_v<TChannel> && is_animatable_type_v<TMember> && is_type_compatible(udm::type_to_enum<TChannel>(),udm::type_to_enum<TMember>()))
static pragma::animation::ChannelValueSubmitter get_member_channel_submitter(pragma::BaseEntityComponent &component,uint32_t memberIdx,void(*setter)(const pragma::ComponentMemberInfo&,pragma::BaseEntityComponent&,const void*,void*),void *userData=nullptr)
{
	return [&component,memberIdx,setter,userData](panima::Channel &channel,uint32_t &inOutPivotTimeIndex,double t) mutable {
		auto *memberInfo = component.GetMemberInfo(memberIdx);
		assert(memberInfo);
		constexpr auto applyValueExpression = [](panima::Channel &channel,auto &inOutValue,double t) {
			if constexpr(std::is_same_v<TChannel,double>)
				channel.ApplyValueExpression(t,inOutValue);
			else if constexpr(std::is_arithmetic_v<TChannel>)
			{
				double dvalue = inOutValue;
				channel.ApplyValueExpression(t,dvalue);
				inOutValue = dvalue; // TODO: Integer rounding?
			}
		};
		if constexpr(std::is_same_v<TChannel,TMember>)
		{
			auto value = channel.GetInterpolatedValue<TChannel>(t,inOutPivotTimeIndex,memberInfo->interpolationFunction);
			if constexpr(std::is_arithmetic_v<TChannel>)
				applyValueExpression(channel,value,t);
			setter(*memberInfo,component,&value,userData);
		}
		else
		{
			// Interpolation function cannot be used unless the type is an exact match
			auto value = channel.GetInterpolatedValue<TChannel>(t,inOutPivotTimeIndex);
			if constexpr(std::is_arithmetic_v<TChannel>)
				applyValueExpression(channel,value,t);
			constexpr auto numChannelComponents = get_component_count(udm::type_to_enum<TChannel>());
			constexpr auto numMemberComponents = get_component_count(udm::type_to_enum<TMember>());
			static_assert(numChannelComponents == TMapArray.size());
			TMember curVal;
			memberInfo->getterFunction(*memberInfo,component,&curVal);
			if constexpr(numChannelComponents == 1)
			{
				if constexpr(numMemberComponents > 1)
					curVal[TMapArray[0]] = value;
				else
					curVal = value;
			}
			else
			{
				// Would be nicer to use a loop, but since there are no constexpr
				// loops as of C++20, we'll just do it manually...
				curVal[TMapArray[0]] = value[0];
				curVal[TMapArray[1]] = value[1];
				if constexpr(numChannelComponents > 2)
				{
					curVal[TMapArray[2]] = value[2];
					if constexpr(numChannelComponents > 3)
						curVal[TMapArray[3]] = value[3];
				}
			}
			setter(*memberInfo,component,&curVal,userData);
		}
	};
}

template<typename TChannel,typename TMember,typename T,uint32_t I,uint32_t ARRAY_INDEX_COUNT,T MAX_ARRAY_VALUE,template<typename,typename,auto TTFunc> class TFunc,T... values>
	static pragma::animation::ChannelValueSubmitter runtime_array_to_compile_time(pragma::BaseEntityComponent &component,uint32_t memberIdx,void(*setter)(const pragma::ComponentMemberInfo&,pragma::BaseEntityComponent&,const void*,void*),void *userData,const std::array<T,ARRAY_INDEX_COUNT> &rtValues);

template<typename TChannel,typename TMember,typename T,uint32_t I,uint32_t VAL,uint32_t ARRAY_INDEX_COUNT,T MAX_ARRAY_VALUE,template<typename,typename,auto TTFunc> class TFunc,T... values>
	static pragma::animation::ChannelValueSubmitter runtime_array_to_compile_time_it(pragma::BaseEntityComponent &component,uint32_t memberIdx,void(*setter)(const pragma::ComponentMemberInfo&,pragma::BaseEntityComponent&,const void*,void*),void *userData,const std::array<T,ARRAY_INDEX_COUNT> &rtValues)
{
    if(rtValues[I] == VAL)
		return runtime_array_to_compile_time<TChannel,TMember,T,I +1,ARRAY_INDEX_COUNT,MAX_ARRAY_VALUE,TFunc,values...,VAL>(component,memberIdx,setter,userData,rtValues);
    else
    {
        if constexpr(VAL <= MAX_ARRAY_VALUE)
            return runtime_array_to_compile_time_it<TChannel,TMember,T,I,VAL +1,ARRAY_INDEX_COUNT,MAX_ARRAY_VALUE,TFunc,values...>(component,memberIdx,setter,userData,rtValues);
    }
	return nullptr;
}

template<typename TChannel,typename TMember,typename T,uint32_t I,uint32_t ARRAY_INDEX_COUNT,T MAX_ARRAY_VALUE,template<typename,typename,auto TTFunc> class TFunc,T... values>
	pragma::animation::ChannelValueSubmitter runtime_array_to_compile_time(pragma::BaseEntityComponent &component,uint32_t memberIdx,void(*setter)(const pragma::ComponentMemberInfo&,pragma::BaseEntityComponent&,const void*,void*),void *userData,const std::array<T,ARRAY_INDEX_COUNT> &rtValues)
	{
		if constexpr(I < ARRAY_INDEX_COUNT)
			return runtime_array_to_compile_time_it<TChannel,TMember,T,I,0,ARRAY_INDEX_COUNT,MAX_ARRAY_VALUE,TFunc,values...>(component,memberIdx,setter,userData,rtValues);
        else
			return TFunc<TChannel,TMember,std::array<T,ARRAY_INDEX_COUNT>{values...}>{}(component,memberIdx,setter,userData);
	}

template<typename TChannel,typename TMember,auto TMapArray>
struct get_member_channel_submitter_wrapper {
    pragma::animation::ChannelValueSubmitter operator()(pragma::BaseEntityComponent &component,uint32_t memberIdx,void(*setter)(const pragma::ComponentMemberInfo&,pragma::BaseEntityComponent&,const void*,void*),void *userData) const
	{
		return get_member_channel_submitter<TChannel,TMember,TMapArray>(component,memberIdx,setter,userData);
	}
};

void Animated2Component::InitializeAnimationChannelValueSubmitters()
{
	for(auto &animManager : m_animationManagers)
		InitializeAnimationChannelValueSubmitters(*animManager);
}

void Animated2Component::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(AnimationDriverComponent))
		m_driverComponent = &static_cast<AnimationDriverComponent&>(component);
}
void Animated2Component::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(AnimationDriverComponent))
		m_driverComponent = nullptr;
}

void Animated2Component::InitializeAnimationChannelValueSubmitters(animation::AnimationManager &manager)
{
	auto *anim = manager.GetCurrentAnimation();
	auto &channelValueSubmitters = manager.GetChannelValueSubmitters();
	if(!anim)
	{
		channelValueSubmitters.clear();
		return;
	}
	auto *driverC = GetDriverComponent();
	auto &channels = anim->GetChannels();
	channelValueSubmitters.resize(channels.size());
	// TODO: Reload this when animation has changed, or if component data has changed (e.g. animated component has changed model and therefore bone positions and rotational data)
	for(auto it=channels.begin();it!=channels.end();++it)
	{
		auto &channel = *it;
		auto &path = channel->targetPath;
		/*if(path.GetFront() != "ec") // First path component denotes the type, which always has to be 'ec' for entity component in this case
			continue;
		path.PopFront();*/
		auto componentTypeName = path.GetFront();
		// TODO: Needs to be updated whenever a new component has been added to the entity
		auto hComponent = GetEntity().FindComponent(componentTypeName);
		if(hComponent.expired())
			continue;
		auto localPath = path;
		localPath.PopFront();
		if(localPath.IsEmpty())
			continue;
		auto channelIdx = it -channels.begin();
		CEAnim2InitializeChannelValueSubmitter evData {localPath};
		if(hComponent->InvokeEventCallbacks(EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER,evData) == util::EventReply::Handled)
		{
			if(evData.submitter == nullptr)
				continue;
			channelValueSubmitters[channelIdx] = std::move(evData.submitter);
			continue;
		}

		auto memberIdx = hComponent->GetMemberIndex(localPath.GetString());
		if(memberIdx.has_value())
			localPath = {};
		else
		{
			auto component = localPath.GetBack();
			localPath.PopBack();
			auto memberName = localPath.GetString();
			if(!memberName.empty())
				memberName.pop_back();
			memberIdx = hComponent->GetMemberIndex(memberName);
			if(!memberIdx.has_value())
				continue;
			localPath = component;
		}
		auto channelValueType = channel->GetValueType();
		auto *memberInfo = hComponent->GetMemberInfo(*memberIdx);
		auto valueType = memberInfo->type;

		ValueDriver *driver = nullptr;
		if(driverC)
		{
			driver = driverC->FindDriver(hComponent->GetComponentId(),*memberIdx);
			//if(driver && (!driver->dataValue.data || driver->dataValue.type != memberInfo->type))
			//	continue;
		}
		
		auto &component = *hComponent;
		auto vsGetMemberChannelSubmitter = [&localPath,&memberIdx,channelIdx,&channelValueSubmitters,&component,driver]<typename TMember>(auto tag) mutable {
			using TChannel = decltype(tag)::type;
			auto strValueComponent = localPath.GetFront();
			constexpr auto setMemberValue = [](const pragma::ComponentMemberInfo &memberInfo,pragma::BaseEntityComponent &component,const void *value,void *userData) {
				memberInfo.setterFunction(memberInfo,component,value);
			};
			constexpr auto setDriverValue = [](const pragma::ComponentMemberInfo &memberInfo,pragma::BaseEntityComponent &component,const void *value,void *userData) {
				//memcpy(static_cast<ValueDriver*>(userData)->dataValue.data.get(),value,sizeof(TMember));// TODO
			};
			if(strValueComponent.empty())
			{
				if constexpr(std::is_same_v<TChannel,TMember>)
				{
					// If the channel has no driver, we can apply the value directly to maximize performance.
					// Otherwise we have to relay the value to the driver first, which will then apply it when invoked later.
					// This is because the driver may manipulate the value before it is actually applied.
					if(!driver)
						channelValueSubmitters[channelIdx] = get_member_channel_submitter<TChannel,TMember,0>(component,*memberIdx,setMemberValue);
					else
						channelValueSubmitters[channelIdx] = get_member_channel_submitter<TChannel,TMember,0>(component,*memberIdx,setDriverValue,driver);
				}
				return;
			}
			constexpr auto channelType = udm::type_to_enum<TChannel>();
			constexpr auto memberType = udm::type_to_enum<TMember>();
			constexpr auto numComponentsChannel = get_component_count(channelType);
			constexpr auto numComponentsMember = get_component_count(memberType);
			if constexpr(numComponentsChannel > 0 && numComponentsMember > 0 && is_type_compatible(channelType,memberType))
			{
				std::vector<std::string> components;
				ustring::explode(strValueComponent,",",components);
				if(components.empty() || components.size() > numComponentsChannel)
					return;
				std::array<uint32_t,numComponentsChannel> componentIndices;
				for(uint32_t idx = 0; auto &strComponent : components)
				{
					switch(memberType)
					{
					case udm::Type::Vector2:
					case udm::Type::Vector2i:
					{
						if(strComponent == "x")
							componentIndices[idx] = 0;
						else if(strComponent == "y")
							componentIndices[idx] = 1;
						else return; // Unknown component type
						break;
					}
					case udm::Type::Vector3:
					case udm::Type::Vector3i:
					{
						if(strComponent == "x")
							componentIndices[idx] = 0;
						else if(strComponent == "y")
							componentIndices[idx] = 1;
						else if(strComponent == "z")
							componentIndices[idx] = 2;
						else return; // Unknown component type
						break;
					}
					case udm::Type::Vector4:
					case udm::Type::Vector4i:
					{
						if(strComponent == "x")
							componentIndices[idx] = 0;
						else if(strComponent == "y")
							componentIndices[idx] = 1;
						else if(strComponent == "z")
							componentIndices[idx] = 2;
						else if(strComponent == "w")
							componentIndices[idx] = 3;
						else return; // Unknown component type
						break;
					}
					case udm::Type::Quaternion:
					{
						if(strComponent == "w")
							componentIndices[idx] = 0;
						else if(strComponent == "x")
							componentIndices[idx] = 1;
						else if(strComponent == "y")
							componentIndices[idx] = 2;
						else if(strComponent == "z")
							componentIndices[idx] = 3;
						else return; // Unknown component type
						break;
					}
					case udm::Type::EulerAngles:
					{
						if(strComponent == "p")
							componentIndices[idx] = 0;
						else if(strComponent == "y")
							componentIndices[idx] = 1;
						else if(strComponent == "r")
							componentIndices[idx] = 2;
						else return; // Unknown component type
						break;
					}
					}
					++idx;
				}
				if(!driver)
				{
					channelValueSubmitters[channelIdx] = runtime_array_to_compile_time<
						TChannel,TMember,decltype(componentIndices)::value_type,0,componentIndices.size(),numComponentsMember,get_member_channel_submitter_wrapper
					>(component,*memberIdx,setMemberValue,nullptr,componentIndices);
				}
				else
				{
					channelValueSubmitters[channelIdx] = runtime_array_to_compile_time<
						TChannel,TMember,decltype(componentIndices)::value_type,0,componentIndices.size(),numComponentsMember,get_member_channel_submitter_wrapper
					>(component,*memberIdx,setDriverValue,driver,componentIndices);
				}
			}
		};

		auto vs = [&vsGetMemberChannelSubmitter,channelValueType](auto tag) mutable {
			using TMember = decltype(tag)::type;
			if constexpr(is_animatable_type_v<TMember>)
			{
				auto vs = [&vsGetMemberChannelSubmitter](auto tag) {
					using TChannel = decltype(tag)::type;
					if constexpr(is_animatable_type_v<TChannel>)
						vsGetMemberChannelSubmitter.template operator()<TMember>(tag);
				};
				if(udm::is_ng_type(channelValueType))
					udm::visit_ng(channelValueType,vs);
			}
		};
		if(udm::is_ng_type(valueType))
			udm::visit_ng(valueType,vs);
	}
}

void Animated2Component::PlayAnimation(animation::AnimationManager &manager,panima::Animation &anim)
{
	manager->SetAnimation(anim);
	InitializeAnimationChannelValueSubmitters(manager);
}
void Animated2Component::ClearAnimationManagers()
{
	m_animationManagers.clear();
}
bool Animated2Component::MaintainAnimations(double dt)
{
	CEAnim2MaintainAnimations evData{dt};
	if(InvokeEventCallbacks(EVENT_MAINTAIN_ANIMATIONS,evData) == util::EventReply::Handled)
	{
		InvokeEventCallbacks(EVENT_ON_ANIMATIONS_UPDATED);
		return true;
	}

	AdvanceAnimations(dt);
	InvokeEventCallbacks(EVENT_ON_ANIMATIONS_UPDATED);
	return true;
}

void Animated2Component::OnTick(double dt)
{
	BaseEntityComponent::OnTick(dt);

	MaintainAnimations(dt);
}
void Animated2Component::AdvanceAnimations(double dt)
{
	auto &ent = GetEntity();
	auto pTimeScaleComponent = ent.GetTimeScaleComponent();
	dt *= (pTimeScaleComponent.valid() ? pTimeScaleComponent->GetEffectiveTimeScale() : 1.f);
	dt *= GetPlaybackRate();
	for(auto &manager : m_animationManagers)
	{
		auto change = (*manager)->Advance(dt);
		if(!change)
			continue;
		auto *anim = manager->GetCurrentAnimation();
		if(!anim)
			continue;
		auto &channelValueSubmitters = manager->GetChannelValueSubmitters();
		auto &channels = anim->GetChannels();
		auto n = umath::min(channelValueSubmitters.size(),channels.size());
		auto t = (*manager)->GetCurrentTime();
		for(auto i=decltype(n){0u};i<n;++i)
		{
			auto &submitter = channelValueSubmitters[i];
			if(!submitter)
				continue;
			auto &channel = channels[i];
			submitter(*channel,(*manager)->GetLastChannelTimestampIndex(i),t);
		}
	}
}
void Animated2Component::InitializeLuaObject(lua_State *l) {pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

void Animated2Component::Initialize()
{
	BaseEntityComponent::Initialize();
	SetTickPolicy(TickPolicy::WhenVisible);

	BindEventUnhandled(BaseEntityComponent::EVENT_ON_MEMBERS_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		InitializeAnimationChannelValueSubmitters();
	});
}

void Animated2Component::Save(udm::LinkedPropertyWrapperArg udm) {}
void Animated2Component::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) {}
void Animated2Component::ResetAnimation(const std::shared_ptr<Model> &mdl) {}

/////////////////

CEAnim2MaintainAnimations::CEAnim2MaintainAnimations(double deltaTime)
	: deltaTime{deltaTime}
{}
void CEAnim2MaintainAnimations::PushArguments(lua_State *l)
{
	Lua::PushNumber(l,deltaTime);
}

/////////////////

CEAnim2TranslateAnimation::CEAnim2TranslateAnimation(panima::AnimationId &animation,pragma::FPlayAnim &flags)
	: animation(animation),flags(flags)
{}
void CEAnim2TranslateAnimation::PushArguments(lua_State *l)
{
	Lua::PushInt(l,animation);
	Lua::PushInt(l,umath::to_integral(flags));
}
uint32_t CEAnim2TranslateAnimation::GetReturnCount() {return 2;}
void CEAnim2TranslateAnimation::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-2))
		animation = Lua::CheckInt(l,-2);
	if(Lua::IsSet(l,-1))
		flags = static_cast<pragma::FPlayAnim>(Lua::CheckInt(l,-1));
}

/////////////////

CEAnim2OnAnimationStart::CEAnim2OnAnimationStart(int32_t animation,Activity activity,pragma::FPlayAnim flags)
	: animation(animation),activity(activity),flags(flags)
{}
void CEAnim2OnAnimationStart::PushArguments(lua_State *l)
{
	Lua::PushInt(l,animation);
	Lua::PushInt(l,umath::to_integral(activity));
	Lua::PushInt(l,umath::to_integral(flags));
}

/////////////////

CEAnim2OnAnimationComplete::CEAnim2OnAnimationComplete(int32_t animation,Activity activity)
	: animation(animation),activity(activity)
{}
void CEAnim2OnAnimationComplete::PushArguments(lua_State *l)
{
	Lua::PushInt(l,animation);
	Lua::PushInt(l,umath::to_integral(activity));
}

/////////////////

CEAnim2HandleAnimationEvent::CEAnim2HandleAnimationEvent(const AnimationEvent &animationEvent)
	: animationEvent(animationEvent)
{}
void CEAnim2HandleAnimationEvent::PushArguments(lua_State *l)
{
	Lua::PushInt(l,static_cast<int32_t>(animationEvent.eventID));

	auto tArgs = Lua::CreateTable(l);
	auto &args = animationEvent.arguments;
	for(auto i=decltype(args.size()){0};i<args.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushString(l,args.at(i));
		Lua::SetTableValue(l,tArgs);
	}
}
void CEAnim2HandleAnimationEvent::PushArgumentVariadic(lua_State *l)
{
	auto &args = animationEvent.arguments;
	for(auto &arg : args)
		Lua::PushString(l,arg);
}

/////////////////

CEAnim2OnPlayAnimation::CEAnim2OnPlayAnimation(panima::AnimationId animation,pragma::FPlayAnim flags)
	: animation(animation),flags(flags)
{}
void CEAnim2OnPlayAnimation::PushArguments(lua_State *l)
{
	Lua::PushInt(l,animation);
	Lua::PushInt(l,umath::to_integral(flags));
}

/////////////////

CEAnim2InitializeChannelValueSubmitter::CEAnim2InitializeChannelValueSubmitter(util::Path &path)
	: path{path}
{}
void CEAnim2InitializeChannelValueSubmitter::PushArguments(lua_State *l) {}
uint32_t CEAnim2InitializeChannelValueSubmitter::GetReturnCount() {return 0;}
void CEAnim2InitializeChannelValueSubmitter::HandleReturnValues(lua_State *l)
{
	
}
#pragma optimize("",on)
