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
#include "pragma/lua/types/udm.hpp"
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/model/model.h"
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/policies/shared_from_this_policy.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <luainterface.hpp>
#include <panima/pose.hpp>
#include <panima/skeleton.hpp>
#include <panima/channel.hpp>
#include <panima/channel_t.hpp>
#include <panima/player.hpp>
#include <panima/animation.hpp>
#include <panima/animation_set.hpp>
#include <panima/animation_manager.hpp>
#include <panima/slice.hpp>
#include <luabind/copy_policy.hpp>

namespace Lua::animation
{
	void register_library(Lua::Interface &lua);
};

void Lua::animation::register_library(Lua::Interface &lua)
{
	auto animMod = luabind::module(lua.GetState(),"panima");

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
	
	auto cdTimeFrame = luabind::class_<panima::TimeFrame>("TimeFrame");
	cdTimeFrame.def(luabind::tostring(luabind::self));
	cdTimeFrame.def_readwrite("startOffset",&panima::TimeFrame::startOffset);
	cdTimeFrame.def_readwrite("scale",&panima::TimeFrame::scale);
	cdTimeFrame.def_readwrite("duration",&panima::TimeFrame::duration);
	animMod[cdTimeFrame];
	pragma::lua::define_custom_constructor<panima::TimeFrame,[](float startOffset,float scale,float duration) -> panima::TimeFrame {
		return panima::TimeFrame{startOffset,scale,duration};
	},float,float,float>(lua.GetState());
	pragma::lua::define_custom_constructor<panima::TimeFrame,[]() -> panima::TimeFrame {
		return panima::TimeFrame{};
	}>(lua.GetState());

	auto cdChannel = luabind::class_<panima::Channel>("Channel");

	auto cdPath = luabind::class_<panima::ChannelPath>("Path");
	cdPath.def(luabind::constructor<>());
	cdPath.def(luabind::constructor<const std::string&>());
	cdPath.def(luabind::tostring(luabind::self));
	cdPath.def_readwrite("path",&panima::ChannelPath::path);
	cdPath.property("components",static_cast<std::vector<std::string>*(panima::ChannelPath::*)()>(&panima::ChannelPath::GetComponents));
	cdPath.def("ToUri",&panima::ChannelPath::ToUri);
	cdPath.def("ToUri",&panima::ChannelPath::ToUri,luabind::default_parameter_policy<2,true>{});
	cdChannel.scope[cdPath];

	cdChannel.def(luabind::tostring(luabind::self));
	cdChannel.def(luabind::const_self ==luabind::const_self);
	cdChannel.def("Update",&panima::Channel::Update);
	cdChannel.def("GetTimeFrame",static_cast<panima::TimeFrame&(panima::Channel::*)()>(&panima::Channel::GetTimeFrame));
	cdChannel.def("SetTimeFrame",&panima::Channel::SetTimeFrame);
	cdChannel.def("GetValueType",&panima::Channel::GetValueType);
	cdChannel.def("SetValueType",&panima::Channel::SetValueType);
	cdChannel.def("GetInterpolation",+[](lua_State *l,panima::Channel &channel) -> panima::ChannelInterpolation {
		return channel.interpolation;
	});
	cdChannel.def("SetInterpolation",+[](lua_State *l,panima::Channel &channel,panima::ChannelInterpolation interp) {
		channel.interpolation = interp;
	});
	cdChannel.def("GetTargetPath",+[](lua_State *l,panima::Channel &channel) -> panima::ChannelPath* {
		return &channel.targetPath;
	});
	cdChannel.def("SetTargetPath",+[](lua_State *l,panima::Channel &channel,const std::string &path) {
		channel.targetPath = path;
	});
	cdChannel.def("GetTimesArray",+[](lua_State *l,panima::Channel &channel) -> ::udm::LinkedPropertyWrapper {
			return ::udm::LinkedPropertyWrapper{channel.GetTimesProperty()};
	});
	cdChannel.def("GetValueArray",+[](lua_State *l,panima::Channel &channel) -> ::udm::LinkedPropertyWrapper {
			return ::udm::LinkedPropertyWrapper{channel.GetValueProperty()};
	});
	cdChannel.def("Resize",&panima::Channel::Resize);
	cdChannel.def("GetSize",&panima::Channel::GetSize);
	cdChannel.def("AddValue",+[](lua_State *l,panima::Channel &channel,float t,Lua::udm_ng value) -> uint32_t {
		return ::udm::visit_ng(channel.GetValueType(),[&channel,t,&value](auto tag) {
			using T = decltype(tag)::type;
			auto v = luabind::object_cast<T>(luabind::object{value});
			return channel.AddValue(t,v);
		});
	});
	cdChannel.def("Save",&panima::Channel::Save);
	cdChannel.def("Load",&panima::Channel::Load);
	cdChannel.def("RemoveValue",+[](lua_State *l,panima::Channel &channel,uint32_t idx) -> bool {
		auto &times = channel.GetTimesArray();
		auto &values = channel.GetValueArray();
		if(idx >= times.GetSize() || idx >= values.GetSize())
			return false;
		times.RemoveValue(idx);
		values.RemoveValue(idx);
		channel.Update();
		return true;
	});
	cdChannel.def("SetTime",+[](lua_State *l,panima::Channel &channel,uint32_t idx,float time) -> bool {
		auto r = Lua::udm::set_array_value(l,channel.GetTimesArray(),idx,luabind::object{l,time});
		channel.Update();
		return r;
	});
	cdChannel.def("SetValue",+[](lua_State *l,panima::Channel &channel,uint32_t idx,const luabind::object &value) -> bool {
		auto r = Lua::udm::set_array_value(l,channel.GetValueArray(),idx,value);
		channel.Update();
		return r;
	});
	cdChannel.def("SetValues",+[](lua_State *l,panima::Channel &channel,luabind::tableT<float> times,luabind::tableT<void> values) {
		auto numTimes = Lua::GetObjectLength(l,times);
		auto numValues = Lua::GetObjectLength(l,values);
		if(numTimes != numValues)
			throw std::runtime_error{"Number of elements in times array (" +std::to_string(numTimes) +") doesn't match number of values in values array (" +std::to_string(numValues) +")! This is not allowed."};
		Lua::udm::set_array_values(l,channel.GetTimesArray(),times,2);
		Lua::udm::set_array_values(l,channel.GetValueArray(),values,3);
		channel.Update();
	});
	cdChannel.def("SetValues",+[](lua_State *l,panima::Channel &channel,luabind::tableT<void> timeValueMap) {
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
		channel.Update();
	});
	cdChannel.def("SetValueExpression",+[](lua_State *l,panima::Channel &channel,std::string expression) -> Lua::var<bool,std::string> {
		std::string err;
		auto res = channel.SetValueExpression(std::move(expression),err);
		if(res)
			return luabind::object{l,true};
		return luabind::object{l,err};
	});
	cdChannel.def("ClearValueExpression",&panima::Channel::ClearValueExpression);
	cdChannel.def("GetValueExpression",+[](panima::Channel &channel) -> std::optional<std::string> {
		auto *expr = channel.GetValueExpression();
		return expr ? *expr : std::optional<std::string>{};
	});
	cdChannel.def("GetValueCount",&panima::Channel::GetValueCount);
	cdChannel.def("GetTime",&panima::Channel::GetTime);
	cdChannel.def("GetValue",+[](lua_State *l,panima::Channel &channel,uint32_t idx) -> Lua::udm_ng {
		if(idx >= channel.GetValueCount())
			return Lua::nil;
		return ::udm::visit_ng(channel.GetValueType(),[l,&channel,idx](auto tag) {
			using T = decltype(tag)::type;
			auto value = channel.GetValue<T>(idx);
			return luabind::object{l,value};
		});
	});
	cdChannel.def("GetTimes",+[](lua_State *l,panima::Channel &channel) -> Lua::tb<float> {
		auto &a = channel.GetTimesArray();
		auto t = luabind::newtable(l);
		auto c = a.GetSize();
		for(auto i=decltype(c){0u};i<c;++i)
			t[i +1] = a.GetValue<float>(i);
		return t;
	});
	cdChannel.def("GetValues",+[](lua_State *l,panima::Channel &channel) -> Lua::tb<void> {
		auto &a = channel.GetValueArray();
		auto t = luabind::newtable(l);
		auto c = a.GetSize();
		::udm::visit_ng(channel.GetValueType(),[l,&channel,c,&a,&t](auto tag) {
			using T = decltype(tag)::type;
			for(auto i=decltype(c){0u};i<c;++i)
				t[i +1] = a.GetValue<T>(i);
		});
		return t;
	});
	animMod[cdChannel];

	auto cdSet = luabind::class_<panima::AnimationSet>("Set");
	cdSet.def(luabind::tostring(luabind::self));
	cdSet.def(luabind::const_self ==luabind::const_self);
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
	cdPlayer.def(luabind::const_self ==luabind::const_self);
	cdPlayer.scope[luabind::def("create",static_cast<std::shared_ptr<panima::Player>(*)()>(&panima::Player::Create))];
	cdPlayer.def("Advance",&panima::Player::Advance);
	cdPlayer.def("Advance",&panima::Player::Advance,luabind::default_parameter_policy<3,false>{});
	cdPlayer.def("GetDuration",&panima::Player::GetDuration);
	cdPlayer.def("GetRemainingDuration",&panima::Player::GetRemainingAnimationDuration);
	cdPlayer.def("GetCurrentTimeFraction",&panima::Player::GetCurrentTimeFraction);
	cdPlayer.def("GetCurrentTime",&panima::Player::GetCurrentTime);
	cdPlayer.def("GetPlaybackRate",&panima::Player::GetPlaybackRate);
	cdPlayer.def("SetPlaybackRate",&panima::Player::SetPlaybackRate);
	cdPlayer.def("SetAnimationDirty",&panima::Player::SetAnimationDirty);
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
	cdManager.def(luabind::const_self ==luabind::const_self);
	cdManager.scope[luabind::def("create",static_cast<std::shared_ptr<panima::AnimationManager>(*)()>(&panima::AnimationManager::Create))];
	cdManager.def("GetPreviousSlice",+[](lua_State *l,panima::AnimationManager &manager) {
		return &manager.GetPreviousSlice();
	});
	cdManager.def("GetCurrentAnimationId",&panima::AnimationManager::GetCurrentAnimationId);
	cdManager.def("AddAnimationSet",&panima::AnimationManager::AddAnimationSet);
	cdManager.def("GetCurrentAnimation",+[](lua_State *l,panima::AnimationManager &manager) -> luabind::optional<std::shared_ptr<panima::Animation>> {
		auto *anim = manager.GetCurrentAnimation();
		return anim ? opt<std::shared_ptr<panima::Animation>>{l,anim->shared_from_this()} : nil;
	});
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
	cdSlice.def("GetChannelValueCount",+[](lua_State *l,panima::Slice &slice) -> uint32_t {
		return slice.channelValues.size();
	});
	cdSlice.def("GetChannelProperty",+[](lua_State *l,panima::Slice &slice,uint32_t idx) -> luabind::optional<::udm::PProperty> {
		if(idx >= slice.channelValues.size())
			return nil;
		return {l,slice.channelValues[idx]};
	});
	animMod[cdSlice];

	auto cdAnim2 = luabind::class_<panima::Animation>("Animation");
	cdAnim2.def(luabind::tostring(luabind::self));
	cdAnim2.def(luabind::const_self ==luabind::const_self);
	cdAnim2.scope[luabind::def("create",+[](lua_State *l) {
		return std::make_shared<panima::Animation>();
	})];
	cdAnim2.scope[luabind::def("load",+[](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> Lua::var<bool,std::shared_ptr<panima::Animation>> {
		auto anim = std::make_shared<panima::Animation>();
		if(anim->Load(prop) == false)
			return luabind::object{l,false};
		return luabind::object{l,anim};
	})];
	cdAnim2.def("GetChannelCount",&panima::Animation::GetChannelCount);
	cdAnim2.def("GetAnimationSpeedFactor",&panima::Animation::GetAnimationSpeedFactor);
	cdAnim2.def("SetAnimationSpeedFactor",&panima::Animation::SetAnimationSpeedFactor);
	cdAnim2.def("GetDuration",&panima::Animation::GetDuration);
	cdAnim2.def("SetDuration",&panima::Animation::SetDuration);
	cdAnim2.def("UpdateDuration",+[](lua_State *l,panima::Animation &anim) -> float {
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
	});
	cdAnim2.def("RemoveChannel",&panima::Animation::RemoveChannel);
	cdAnim2.def("AddChannel",static_cast<void(panima::Animation::*)(panima::Channel&)>(&panima::Animation::AddChannel));
	cdAnim2.def("AddChannel",+[](lua_State *l,panima::Animation &anim,const std::string &path,::udm::Type valueType) -> opt<std::shared_ptr<panima::Channel>> {
		auto *channel = anim.AddChannel(path,valueType);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	});
	cdAnim2.def("GetChannels",+[](lua_State *l,panima::Animation &anim) -> luabind::tableT<panima::Channel> {
		return Lua::vector_to_table<std::shared_ptr<panima::Channel>>(l,anim.GetChannels());
	});
	cdAnim2.def("FindChannel",+[](lua_State *l,panima::Animation &anim,std::string path) -> opt<std::shared_ptr<panima::Channel>> {
		auto *channel = anim.FindChannel(std::move(path));
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	});
	cdAnim2.def("Save",&panima::Animation::Save);
	cdAnim2.scope[luabind::def("Load",+[](lua_State *l,::udm::LinkedPropertyWrapper &assetData) -> std::shared_ptr<panima::Animation> {
		auto anim = std::make_shared<panima::Animation>();
		if(anim->Load(assetData) == false)
			return nullptr;
		return anim;
	})];
	animMod[cdAnim2];
}
