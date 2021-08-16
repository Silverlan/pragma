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
#include "pragma/model/model.h"
#include "pragma/model/animation/animation_manager.hpp"
#include "pragma/model/animation/animation.hpp"
#include "pragma/model/animation/animation2.hpp"
#include "pragma/model/animation/animation_channel.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/lua_call.hpp"

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
	callbackInteface.onPlayAnimation = [this](animation::AnimationId animId,FPlayAnim flags) -> bool {
		CEAnim2OnPlayAnimation evData{animId,flags};
		return InvokeEventCallbacks(EVENT_PLAY_ANIMATION,evData) != util::EventReply::Handled;
	};
	callbackInteface.onStopAnimation = []() {
	
	};
	callbackInteface.translateAnimation = [this](animation::AnimationId &animId,FPlayAnim &flags) {
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
void Animated2Component::PlayAnimation(animation::AnimationManager &manager,pragma::animation::Animation2 &anim)
{
	manager->SetAnimation(anim);
	auto &channels = anim.GetChannels();
	auto &channelValueSubmitters = manager.GetChannelValueSubmitters();
	channelValueSubmitters.resize(channels.size());
	// TODO: Reload this when animation has changed, or if component data has changed (e.g. animated component has changed model and therefore bone positions and rotational data)
	for(auto it=channels.begin();it!=channels.end();++it)
	{
		auto &channel = *it;
		auto &path = channel->targetPath;
		auto componentTypeName = path.GetFront();
		// TODO: Needs to be updated whenever a new component has been added to the entity
		auto hComponent = GetEntity().FindComponent(componentTypeName);
		if(hComponent.expired())
			continue;
		auto localPath = path;
		localPath.PopFront();

		CEAnim2InitializeChannelValueSubmitter evData {localPath};
		if(hComponent->InvokeEventCallbacks(EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER,evData) != util::EventReply::Handled || evData.submitter == nullptr)
			continue;
		channelValueSubmitters[it -channels.begin()] = std::move(evData.submitter);
	}
}
void Animated2Component::ClearAnimationManagers()
{
	m_animationManagers.clear();
}
bool Animated2Component::MaintainAnimations(double dt)
{
	BaseEntityComponent::OnTick(dt);
	
	auto &ent = GetEntity();
	auto pTimeScaleComponent = ent.GetTimeScaleComponent();
	dt *= pTimeScaleComponent.valid() ? pTimeScaleComponent->GetEffectiveTimeScale() : 1.f;
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

	auto &ent = GetEntity();
	auto pTimeScaleComponent = ent.GetTimeScaleComponent();
	MaintainAnimations(dt *(pTimeScaleComponent.valid() ? pTimeScaleComponent->GetEffectiveTimeScale() : 1.f));
}
void Animated2Component::AdvanceAnimations(double dt)
{
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

CEAnim2TranslateAnimation::CEAnim2TranslateAnimation(animation::AnimationId &animation,pragma::FPlayAnim &flags)
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

CEAnim2OnPlayAnimation::CEAnim2OnPlayAnimation(animation::AnimationId animation,pragma::FPlayAnim flags)
	: animation(animation),flags(flags)
{}
void CEAnim2OnPlayAnimation::PushArguments(lua_State *l)
{
	Lua::PushInt(l,animation);
	Lua::PushInt(l,umath::to_integral(flags));
}

/////////////////

CEAnim2InitializeChannelValueSubmitter::CEAnim2InitializeChannelValueSubmitter(const util::Path &path)
	: path{path}
{}
void CEAnim2InitializeChannelValueSubmitter::PushArguments(lua_State *l) {}
uint32_t CEAnim2InitializeChannelValueSubmitter::GetReturnCount() {return 0;}
void CEAnim2InitializeChannelValueSubmitter::HandleReturnValues(lua_State *l)
{
	
}
#pragma optimize("",on)
