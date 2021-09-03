/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/skeletal_animation.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include "pragma/lua/libraries/ludm.hpp"
#include "pragma/model/model.h"
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/policies/shared_from_this_policy.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <luainterface.hpp>
#include <panima/pose.hpp>
#include <panima/skeleton.hpp>
#include <panima/channel.hpp>
#include <panima/player.hpp>
#include <panima/animation.hpp>
#include <panima/animation_set.hpp>
#include <panima/animation_manager.hpp>
#include <panima/slice.hpp>

namespace Lua::animation
{
	void register_library(Lua::Interface &lua);
};
void Lua::animation::register_library(Lua::Interface &lua)
{
	auto animMod = luabind::module(lua.GetState(),"animation");

	auto cdPose = luabind::class_<panima::Pose>("Pose");
	cdPose.def(luabind::tostring(luabind::self));
	cdPose.def(luabind::constructor<>());
	cdPose.def("SetTransformCount",&panima::Pose::SetTransformCount);
	cdPose.def("SetBoneIndex",&panima::Pose::SetBoneIndex);
	cdPose.def("GetTransform",static_cast<umath::ScaledTransform*(panima::Pose::*)(BoneId)>(&panima::Pose::GetTransform));
	cdPose.def("SetTransform",&panima::Pose::SetTransform);
	cdPose.def("Clear",&panima::Pose::Clear);
	cdPose.def("Lerp",&panima::Pose::Lerp);
	cdPose.def("Localize",&panima::Pose::Localize);
	cdPose.def("Globalize",&panima::Pose::Globalize);
	cdPose.def("GetBoneTranslationTable",static_cast<std::vector<uint32_t>&(panima::Pose::*)()>(&panima::Pose::GetBoneTranslationTable));
	animMod[cdPose];

	auto cdChannel = luabind::class_<panima::Channel>("Channel");
	cdChannel.def(luabind::tostring(luabind::self));
	cdChannel.def("GetValueType",&panima::Channel::GetValueType);
	cdChannel.def("SetValueType",&panima::Channel::SetValueType);
	cdChannel.def("GetInterpolation",static_cast<panima::ChannelInterpolation(*)(lua_State*,panima::Channel&)>(
		[](lua_State *l,panima::Channel &channel) -> panima::ChannelInterpolation {
		return channel.interpolation;
	}));
	cdChannel.def("SetInterpolation",static_cast<void(*)(lua_State*,panima::Channel&,panima::ChannelInterpolation)>(
		[](lua_State *l,panima::Channel &channel,panima::ChannelInterpolation interp) {
		channel.interpolation = interp;
	}));
	cdChannel.def("GetTargetPath",static_cast<util::Path(*)(lua_State*,panima::Channel&)>(
		[](lua_State *l,panima::Channel &channel) -> util::Path {
		return channel.targetPath;
	}));
	cdChannel.def("SetTargetPath",static_cast<void(*)(lua_State*,panima::Channel&,const std::string &path)>(
		[](lua_State *l,panima::Channel &channel,const std::string &path) {
		channel.targetPath = path;
	}));
	cdChannel.def("GetTimesArray",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,panima::Channel&)>(
		[](lua_State *l,panima::Channel &channel) -> ::udm::LinkedPropertyWrapper {
			return ::udm::LinkedPropertyWrapper{channel.GetTimesProperty()};
	}));
	cdChannel.def("GetValueArray",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,panima::Channel&)>(
		[](lua_State *l,panima::Channel &channel) -> ::udm::LinkedPropertyWrapper {
			return ::udm::LinkedPropertyWrapper{channel.GetValueProperty()};
	}));
	cdChannel.def("Save",&panima::Channel::Save);
	cdChannel.def("Load",&panima::Channel::Load);
	cdChannel.def("SetValues",
		static_cast<void(*)(lua_State*,panima::Channel&,luabind::tableT<float>,const luabind::tableT<void>)>(
			[](lua_State *l,panima::Channel &channel,luabind::tableT<float> times,const luabind::tableT<void> values) {
		Lua::udm::set_array_values(l,channel.GetTimesArray(),times,2);
		Lua::udm::set_array_values(l,channel.GetValueArray(),values,3);
	}));
	cdChannel.def("SetValues",
		static_cast<void(*)(lua_State*,panima::Channel&,luabind::tableT<void>)>(
			[](lua_State *l,panima::Channel &channel,luabind::tableT<void> timeValueMap) {
				auto type = channel.GetValueType();
				if(type == ::udm::Type::Invalid)
					return;
				uint32_t count = 0;
				for(luabind::iterator it{timeValueMap},end;it!=end;++it)
					++count;

				std::vector<std::pair<float,luabind::object>> timeValues;
				std::vector<uint32_t> sortedIndices;
				timeValues.resize(count);
				sortedIndices.resize(count);

				uint32_t idx = 0;
				for(luabind::iterator it{timeValueMap},end;it!=end;++it)
				{
					auto &tv = timeValues[idx];
					tv.first = luabind::object_cast<float>(it.key());
					tv.second = luabind::object{*it};
					sortedIndices[idx] = idx;
					++idx;
				}

				std::sort(sortedIndices.begin(),sortedIndices.end(),[&timeValues](uint32_t idx0,uint32_t idx1) -> bool {
					return timeValues[idx0].first < timeValues[idx1].first;
				});

				auto &times = channel.GetTimesArray();
				auto &values = channel.GetValueArray();
				times.Resize(count);
				values.Resize(count);

				for(uint32_t i=0;auto idx : sortedIndices)
				{
					auto &tv = timeValues[idx];
					times[i] = tv.first;
					::udm::visit_ng(type,[&tv,&values,i](auto tag) {
						using T = decltype(tag)::type;
						values[i] = luabind::object_cast<T>(tv.second);
					});
					++i;
				}
	}));
	cdChannel.def("SetValueExpression",static_cast<Lua::var<bool,std::string>(*)(lua_State*,panima::Channel&,std::string)>([](lua_State *l,panima::Channel &channel,std::string expression) -> Lua::var<bool,std::string> {
		std::string err;
		auto res = channel.SetValueExpression(std::move(expression),err);
		if(res)
			return luabind::object{l,true};
		return luabind::object{l,err};
	}));
	cdChannel.def("GetValueExpression",&panima::Channel::GetValueExpression);
	animMod[cdChannel];

	auto cdSet = luabind::class_<panima::AnimationSet>("Set");
	cdSet.def(luabind::tostring(luabind::self));
	cdSet.scope[luabind::def("create",&panima::AnimationSet::Create)];
	cdSet.def("Clear",&panima::AnimationSet::Clear);
	cdSet.def("AddAnimation",&panima::AnimationSet::AddAnimation);
	cdSet.def("RemoveAnimation",static_cast<void(panima::AnimationSet::*)(const std::string_view&)>(&panima::AnimationSet::RemoveAnimation));
	cdSet.def("RemoveAnimation",static_cast<void(panima::AnimationSet::*)(const panima::Animation&)>(&panima::AnimationSet::RemoveAnimation));
	cdSet.def("RemoveAnimation",static_cast<void(panima::AnimationSet::*)(panima::AnimationId)>(&panima::AnimationSet::RemoveAnimation));
	cdSet.def("LookupAnimation",&panima::AnimationSet::LookupAnimation);
	cdSet.def("GetAnimation",static_cast<panima::Animation*(panima::AnimationSet::*)(panima::AnimationId)>(&panima::AnimationSet::GetAnimation),luabind::shared_from_this_policy<0>{});
	cdSet.def("GetAnimations",static_cast<std::vector<std::shared_ptr<panima::Animation>>&(panima::AnimationSet::*)()>(&panima::AnimationSet::GetAnimations));
	cdSet.def("FindAnimation",static_cast<panima::Animation*(panima::AnimationSet::*)(const std::string_view&)>(&panima::AnimationSet::FindAnimation),luabind::shared_from_this_policy<0>{});
	cdSet.def("Reserve",&panima::AnimationSet::Reserve);
	cdSet.def("GetSize",&panima::AnimationSet::GetSize);
	animMod[cdSet];

	auto cdPlayer = luabind::class_<panima::Player>("Player");
	cdPlayer.def(luabind::tostring(luabind::self));
	cdPlayer.scope[luabind::def("create",static_cast<std::shared_ptr<panima::Player>(*)()>(&panima::Player::Create))];
	cdPlayer.def("Advance",&panima::Player::Advance);
	cdPlayer.def("Advance",&panima::Player::Advance,luabind::default_parameter_policy<3,false>{});
	cdPlayer.def("GetDuration",&panima::Player::GetDuration);
	cdPlayer.def("GetRemainingDuration",&panima::Player::GetRemainingAnimationDuration);
	cdPlayer.def("GetCurrentTimeFraction",&panima::Player::GetCurrentTimeFraction);
	cdPlayer.def("GetCurrentTime",&panima::Player::GetCurrentTime);
	cdPlayer.def("GetPlaybackRate",&panima::Player::GetPlaybackRate);
	cdPlayer.def("SetPlaybackRate",&panima::Player::SetPlaybackRate);
	cdPlayer.def("SetCurrentTime",&panima::Player::SetCurrentTime);
	cdPlayer.def("SetCurrentTime",&panima::Player::SetCurrentTime,luabind::default_parameter_policy<3,false>{});
	cdPlayer.def("Reset",&panima::Player::Reset);
	cdPlayer.def("GetCurrentSlice",static_cast<panima::Slice*(*)(lua_State*,panima::Player&)>(
		[](lua_State *l,panima::Player &player) {
		return &player.GetCurrentSlice();
	}));
	cdPlayer.def("SetLooping",&panima::Player::SetLooping);
	cdPlayer.def("IsLooping",&panima::Player::IsLooping);
	cdPlayer.def("SetAnimation",&panima::Player::SetAnimation);
	animMod[cdPlayer];

	auto cdManager = luabind::class_<panima::AnimationManager>("Manager");
	cdManager.def(luabind::tostring(luabind::self));
	cdManager.scope[luabind::def("create",static_cast<std::shared_ptr<panima::AnimationManager>(*)()>(&panima::AnimationManager::Create))];
	cdManager.def("GetPreviousSlice",static_cast<panima::Slice*(*)(lua_State*,panima::AnimationManager&)>(
		[](lua_State *l,panima::AnimationManager &manager) {
		return &manager.GetPreviousSlice();
	}));
	cdManager.def("GetCurrentAnimationId",&panima::AnimationManager::GetCurrentAnimationId);
	cdManager.def("AddAnimationSet",&panima::AnimationManager::AddAnimationSet);
	cdManager.def("GetCurrentAnimation",static_cast<opt<std::shared_ptr<panima::Animation>>(*)(lua_State*,panima::AnimationManager&)>([](lua_State *l,panima::AnimationManager &manager) -> luabind::optional<std::shared_ptr<panima::Animation>> {
		auto *anim = manager.GetCurrentAnimation();
		return anim ? opt<std::shared_ptr<panima::Animation>>{l,anim->shared_from_this()} : nil;
	}));
	cdManager.def("GetPlayer",static_cast<panima::Player&(panima::AnimationManager::*)()>(&panima::AnimationManager::GetPlayer),luabind::shared_from_this_policy<0>{});
	cdManager.def("StopAnimation",static_cast<void(panima::AnimationManager::*)()>(&panima::AnimationManager::StopAnimation));
	cdManager.def("PlayAnimation",static_cast<void(panima::AnimationManager::*)(const std::string&,panima::AnimationId,panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation));
	cdManager.def("PlayAnimation",static_cast<void(panima::AnimationManager::*)(const std::string&,panima::AnimationId,panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation),luabind::default_parameter_policy<4,panima::PlaybackFlags::Default>{});
	cdManager.def("PlayAnimation",static_cast<void(panima::AnimationManager::*)(const std::string&,const std::string&,panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation));
	cdManager.def("PlayAnimation",static_cast<void(panima::AnimationManager::*)(const std::string&,const std::string&,panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation),luabind::default_parameter_policy<4,panima::PlaybackFlags::Default>{});
	cdManager.def("PlayAnimation",static_cast<void(panima::AnimationManager::*)(const std::string&,panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation));
	cdManager.def("PlayAnimation",static_cast<void(panima::AnimationManager::*)(const std::string&,panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation),luabind::default_parameter_policy<4,panima::PlaybackFlags::Default>{});
	animMod[cdManager];

	auto cdSlice = luabind::class_<panima::Slice>("Slice");
	cdSlice.def(luabind::tostring(luabind::self));
	cdSlice.def("GetChannelValueCount",static_cast<uint32_t(*)(lua_State*,panima::Slice&)>([](lua_State *l,panima::Slice &slice) -> uint32_t {
		return slice.channelValues.size();
	}));
	cdSlice.def("GetChannelProperty",static_cast<luabind::optional<::udm::PProperty>(*)(lua_State*,panima::Slice&,uint32_t)>([](lua_State *l,panima::Slice &slice,uint32_t idx) -> luabind::optional<::udm::PProperty> {
		if(idx >= slice.channelValues.size())
			return nil;
		return {l,slice.channelValues[idx]};
	}));
	animMod[cdSlice];

	auto cdAnim2 = luabind::class_<panima::Animation>("Animation2");
	cdAnim2.def(luabind::tostring(luabind::self));
	cdAnim2.scope[luabind::def("create",static_cast<std::shared_ptr<panima::Animation>(*)(lua_State*)>([](lua_State *l) {
		return std::make_shared<panima::Animation>();
	}))];
	cdAnim2.def("GetChannelCount",&panima::Animation::GetChannelCount);
	cdAnim2.def("GetAnimationSpeedFactor",&panima::Animation::GetAnimationSpeedFactor);
	cdAnim2.def("SetAnimationSpeedFactor",&panima::Animation::SetAnimationSpeedFactor);
	cdAnim2.def("GetDuration",&panima::Animation::GetDuration);
	cdAnim2.def("SetDuration",&panima::Animation::SetDuration);
	cdAnim2.def("UpdateDuration",static_cast<float(*)(lua_State*,panima::Animation&)>([](lua_State *l,panima::Animation &anim) -> float {
		auto duration = 0.f;
		for(auto &channel : anim.GetChannels())
		{
			auto time = channel->GetTime(channel->GetTimeCount() -1);
			if(!time.has_value())
				continue;
			duration = umath::max(duration,*time);
		}
		anim.SetDuration(duration);
		return duration;
	}));
	cdAnim2.def("AddChannel",static_cast<void(panima::Animation::*)(panima::Channel&)>(&panima::Animation::AddChannel));
	cdAnim2.def("AddChannel",static_cast<opt<std::shared_ptr<panima::Channel>>(*)(lua_State*,panima::Animation&,const util::Path&,::udm::Type)>([](lua_State *l,panima::Animation &anim,const util::Path &path,::udm::Type valueType) -> opt<std::shared_ptr<panima::Channel>> {
		auto *channel = anim.AddChannel(path,valueType);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	}));
	cdAnim2.def("AddChannel",static_cast<opt<std::shared_ptr<panima::Channel>>(*)(lua_State*,panima::Animation&,const std::string&,::udm::Type)>([](lua_State *l,panima::Animation &anim,const std::string &path,::udm::Type valueType) -> opt<std::shared_ptr<panima::Channel>> {
		auto *channel = anim.AddChannel(path,valueType);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	}));
	cdAnim2.def("GetChannels",static_cast<std::vector<std::shared_ptr<panima::Channel>>&(panima::Animation::*)()>(&panima::Animation::GetChannels));
	cdAnim2.def("FindChannel",static_cast<opt<std::shared_ptr<panima::Channel>>(*)(lua_State*,panima::Animation&,const util::Path&)>([](lua_State *l,panima::Animation &anim,const util::Path &path) -> opt<std::shared_ptr<panima::Channel>> {
		auto *channel = anim.FindChannel(path);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	}));
	cdAnim2.def("Save",&panima::Animation::Save);
	cdAnim2.scope[luabind::def("Load",static_cast<std::shared_ptr<panima::Animation>(*)(lua_State*,::udm::LinkedPropertyWrapper&)>([](lua_State *l,::udm::LinkedPropertyWrapper &assetData) -> std::shared_ptr<panima::Animation> {
		auto anim = std::make_shared<panima::Animation>();
		if(anim->Load(assetData) == false)
			return nullptr;
		return anim;
	}))];
	animMod[cdAnim2];
}
