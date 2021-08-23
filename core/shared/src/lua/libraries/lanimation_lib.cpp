/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animation_manager.hpp"
#include "pragma/model/animation/skeletal_animation.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include "pragma/lua/libraries/ludm.hpp"
#include "pragma/model/model.h"
#include <luainterface.hpp>
#include <panima/pose.hpp>
#include <panima/skeleton.hpp>
#include <panima/channel.hpp>
#include <panima/player.hpp>
#include <panima/animation.hpp>
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
	cdPose.def("SetTransformCount",static_cast<void(*)(lua_State*,panima::Pose&,uint32_t)>([](lua_State *l,panima::Pose &pose,uint32_t count) {
		pose.SetTransformCount(count);
	}));
	cdPose.def("SetBoneIndex",static_cast<void(*)(lua_State*,panima::Pose&,uint32_t,uint32_t)>([](lua_State *l,panima::Pose &pose,uint32_t idx,uint32_t boneIdx) {
		pose.SetBoneIndex(idx,boneIdx);
	}));
	cdPose.def("GetTransform",static_cast<umath::ScaledTransform*(*)(lua_State*,panima::Pose&,uint32_t)>([](lua_State *l,panima::Pose &pose,uint32_t boneIdx) -> umath::ScaledTransform* {
		return pose.GetTransform(boneIdx);
	}));
	cdPose.def("SetTransform",static_cast<void(*)(lua_State*,panima::Pose&,uint32_t,const umath::ScaledTransform&)>(
		[](lua_State *l,panima::Pose &pose,uint32_t boneIdx,const umath::ScaledTransform &transform) {
		pose.SetTransform(boneIdx,transform);
	}));
	cdPose.def("Clear",static_cast<void(*)(lua_State*,panima::Pose&)>(
		[](lua_State *l,panima::Pose &pose) {
		pose.Clear();
	}));
	cdPose.def("Lerp",static_cast<void(*)(lua_State*,panima::Pose&,const panima::Pose&,float)>(
		[](lua_State *l,panima::Pose &pose,const panima::Pose &other,float f) {
		pose.Lerp(other,f);
	}));
	cdPose.def("Localize",static_cast<void(*)(lua_State*,panima::Pose&,const panima::Skeleton&)>(
		[](lua_State *l,panima::Pose &pose,const panima::Skeleton &skeleton) {
		pose.Localize(skeleton);
	}));
	cdPose.def("Globalize",static_cast<void(*)(lua_State*,panima::Pose&,const panima::Skeleton&)>(
		[](lua_State *l,panima::Pose &pose,const panima::Skeleton &skeleton) {
		pose.Globalize(skeleton);
	}));
	cdPose.def("GetBoneTranslationTable",static_cast<luabind::tableT<uint32_t>(*)(lua_State*,panima::Pose&)>(
		[](lua_State *l,panima::Pose &pose) {
		return luabind::tableT<uint32_t>{Lua::vector_to_table(l,pose.GetBoneTranslationTable())};
	}));
	animMod[cdPose];

	auto cdChannel = luabind::class_<panima::Channel>("Channel");
	cdChannel.def(luabind::tostring(luabind::self));
	cdChannel.def("GetValueType",static_cast<::udm::Type(*)(lua_State*,panima::Channel&)>([](lua_State *l,panima::Channel &channel) -> ::udm::Type {
		return channel.GetValueType();
	}));
	cdChannel.def("SetValueType",static_cast<void(*)(lua_State*,panima::Channel&,::udm::Type)>([](lua_State *l,panima::Channel &channel,::udm::Type type) {
		channel.SetValueType(type);
	}));
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
	cdChannel.def("Save",static_cast<bool(*)(lua_State*,panima::Channel&,::udm::LinkedPropertyWrapper&)>(
		[](lua_State *l,panima::Channel &channel,::udm::LinkedPropertyWrapper &prop) -> bool {
		return channel.Save(prop);
	}));
	cdChannel.def("Load",static_cast<bool(*)(lua_State*,panima::Channel&,::udm::LinkedPropertyWrapper&)>(
		[](lua_State *l,panima::Channel &channel,::udm::LinkedPropertyWrapper &prop) -> bool {
		return channel.Load(prop);
	}));
	cdChannel.def("SetValues",
		static_cast<void(*)(lua_State*,panima::Channel&,luabind::tableT<float>,const luabind::tableT<void>)>(
			[](lua_State *l,panima::Channel &channel,luabind::tableT<float> times,const luabind::tableT<void> values) {
		Lua::udm::set_array_values(l,channel.GetTimesArray(),times,2);
		Lua::udm::set_array_values(l,channel.GetValueArray(),values,3);
	}));
	animMod[cdChannel];

	auto cdPlayer = luabind::class_<panima::Player>("Player");
	cdPlayer.def(luabind::tostring(luabind::self));
	cdPlayer.scope[luabind::def("create",static_cast<std::shared_ptr<panima::Player>(*)(lua_State*)>([](lua_State *l) {
		return panima::Player::Create();
	}))];
	cdPlayer.def("Advance",static_cast<void(*)(lua_State*,panima::Player&,float,bool)>([](lua_State *l,panima::Player &player,float dt,bool force) {
		player.Advance(dt,force);
	}));
	cdPlayer.def("Advance",static_cast<void(*)(lua_State*,panima::Player&,float)>([](lua_State *l,panima::Player &player,float dt) {
		player.Advance(dt);
	}));
	cdPlayer.def("GetDuration",static_cast<float(*)(lua_State*,panima::Player&)>([](lua_State *l,panima::Player &player) {
		return player.GetDuration();
	}));
	cdPlayer.def("GetRemainingDuration",static_cast<float(*)(lua_State*,panima::Player&)>([](lua_State *l,panima::Player &player) {
		return player.GetDuration();
	}));
	cdPlayer.def("GetCurrentTimeFraction",static_cast<float(*)(lua_State*,panima::Player&)>([](lua_State *l,panima::Player &player) {
		return player.GetCurrentTimeFraction();
	}));
	cdPlayer.def("GetCurrentTime",static_cast<float(*)(lua_State*,panima::Player&)>([](lua_State *l,panima::Player &player) {
		return player.GetCurrentTime();
	}));
	cdPlayer.def("GetPlaybackRate",static_cast<float(*)(lua_State*,panima::Player&)>([](lua_State *l,panima::Player &player) {
		return player.GetPlaybackRate();
	}));
	cdPlayer.def("SetPlaybackRate",static_cast<void(*)(lua_State*,panima::Player&,float)>([](lua_State *l,panima::Player &player,float playbackRate) {
		player.SetPlaybackRate(playbackRate);
	}));
	cdPlayer.def("SetCurrentTime",static_cast<void(*)(lua_State*,panima::Player&,float)>([](lua_State *l,panima::Player &player,float time) {
		player.SetCurrentTime(time);
	}));
	cdPlayer.def("SetCurrentTime",static_cast<void(*)(lua_State*,panima::Player&,float,bool)>([](lua_State *l,panima::Player &player,float time,bool force) {
		player.SetCurrentTime(time,force);
	}));
	cdPlayer.def("Reset",static_cast<void(*)(lua_State*,panima::Player&)>(
		[](lua_State *l,panima::Player &player) {
		player.Reset();
	}));
	cdPlayer.def("GetCurrentSlice",static_cast<panima::Slice*(*)(lua_State*,panima::Player&)>(
		[](lua_State *l,panima::Player &player) {
		return &player.GetCurrentSlice();
	}));
	cdPlayer.def("SetLooping",static_cast<void(*)(lua_State*,panima::Player&,bool)>(
		[](lua_State *l,panima::Player &player,bool looping) {
		player.SetLooping(looping);
	}));
	cdPlayer.def("IsLooping",static_cast<bool(*)(lua_State*,const panima::Player&)>(
		[](lua_State *l,const panima::Player &player) -> bool {
		return player.IsLooping();
	}));
	cdPlayer.def("SetAnimation",static_cast<void(*)(lua_State*,panima::Player&,panima::Animation&)>(
		[](lua_State *l,panima::Player &player,panima::Animation &anim) {
		player.SetAnimation(anim);
	}));
	animMod[cdPlayer];

	auto cdManager = luabind::class_<pragma::animation::AnimationManager>("Manager");
	cdManager.def(luabind::tostring(luabind::self));
	cdManager.scope[luabind::def("create",static_cast<std::shared_ptr<pragma::animation::AnimationManager>(*)(lua_State*,Model&)>([](lua_State *l,Model &mdl) {
		return pragma::animation::AnimationManager::Create(mdl);
	}))];
	cdManager.def("GetPreviousSlice",static_cast<panima::Slice*(*)(lua_State*,pragma::animation::AnimationManager&)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager) {
		return &manager.GetPreviousSlice();
	}));
	cdManager.def("GetCurrentAnimationId",static_cast<panima::AnimationId(*)(lua_State*,pragma::animation::AnimationManager&)>([](lua_State *l,pragma::animation::AnimationManager &manager) {
		return manager.GetCurrentAnimationId();
	}));
	cdManager.def("GetCurrentAnimation",static_cast<opt<std::shared_ptr<panima::Animation>>(*)(lua_State*,pragma::animation::AnimationManager&)>([](lua_State *l,pragma::animation::AnimationManager &manager) -> luabind::optional<std::shared_ptr<panima::Animation>> {
		auto *anim = manager.GetCurrentAnimation();
		return anim ? opt<std::shared_ptr<panima::Animation>>{l,anim->shared_from_this()} : nil;
	}));
	cdManager.def("GetPlayer",static_cast<panima::PPlayer(*)(lua_State*,pragma::animation::AnimationManager&)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager) -> panima::PPlayer {
		return manager.GetPlayer().shared_from_this();
	}));
	cdManager.def("GetModel",static_cast<opt<std::shared_ptr<Model>>(*)(lua_State*,pragma::animation::AnimationManager&)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager) -> luabind::optional<std::shared_ptr<Model>> {
		auto *mdl = const_cast<Model*>(manager.GetModel());
		return mdl ? opt<std::shared_ptr<Model>>{l,mdl->shared_from_this()} : nil;
	}));
	cdManager.def("StopAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationManager&)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager) {
		manager.StopAnimation();
	}));
	cdManager.def("PlayAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationManager&,panima::AnimationId,pragma::FPlayAnim)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager,panima::AnimationId id,pragma::FPlayAnim flags) {
		manager.PlayAnimation(id,flags);
	}));
	cdManager.def("PlayAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationManager&,panima::AnimationId)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager,panima::AnimationId id) {
		manager.PlayAnimation(id);
	}));
	cdManager.def("PlayAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationManager&,const std::string&,pragma::FPlayAnim)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager,const std::string &anim,pragma::FPlayAnim flags) {
		manager.PlayAnimation(anim,flags);
	}));
	cdManager.def("PlayAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationManager&,const std::string&)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager,const std::string &anim) {
		manager.PlayAnimation(anim);
	}));
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
	cdAnim2.def("GetChannelCount",static_cast<uint32_t(*)(lua_State*,panima::Animation&)>([](lua_State *l,panima::Animation &anim) {
		return anim.GetChannelCount();
	}));
	cdAnim2.def("GetAnimationSpeedFactor",static_cast<float(*)(lua_State*,panima::Animation&)>([](lua_State *l,panima::Animation &anim) {
		return anim.GetAnimationSpeedFactor();
	}));
	cdAnim2.def("SetAnimationSpeedFactor",static_cast<void(*)(lua_State*,panima::Animation&,float)>([](lua_State *l,panima::Animation &anim,float factor) {
		anim.SetAnimationSpeedFactor(factor);
	}));
	cdAnim2.def("GetDuration",static_cast<float(*)(lua_State*,panima::Animation&)>([](lua_State *l,panima::Animation &anim) {
		return anim.GetDuration();
	}));
	cdAnim2.def("SetDuration",static_cast<void(*)(lua_State*,panima::Animation&,float)>([](lua_State *l,panima::Animation &anim,float duration) {
		anim.SetDuration(duration);
	}));
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
	cdAnim2.def("AddChannel",static_cast<void(*)(lua_State*,panima::Animation&,panima::Channel&)>([](lua_State *l,panima::Animation &anim,panima::Channel &channel) {
		anim.AddChannel(channel);
	}));
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
	cdAnim2.def("GetChannels",static_cast<tb<std::shared_ptr<panima::Channel>>(*)(lua_State*,panima::Animation&)>([](lua_State *l,panima::Animation &anim) -> tb<std::shared_ptr<panima::Channel>> {
		return Lua::vector_to_table(l,anim.GetChannels());
	}));
	cdAnim2.def("FindChannel",static_cast<opt<std::shared_ptr<panima::Channel>>(*)(lua_State*,panima::Animation&,const util::Path&)>([](lua_State *l,panima::Animation &anim,const util::Path &path) -> opt<std::shared_ptr<panima::Channel>> {
		auto *channel = anim.FindChannel(path);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	}));
	cdAnim2.def("Save",static_cast<bool(*)(panima::Animation&,::udm::LinkedPropertyWrapper&)>([](panima::Animation &anim,::udm::LinkedPropertyWrapper &assetData) -> bool {
		return anim.Save(assetData);
	}));
	cdAnim2.scope[luabind::def("Load",static_cast<std::shared_ptr<panima::Animation>(*)(lua_State*,::udm::LinkedPropertyWrapper&)>([](lua_State *l,::udm::LinkedPropertyWrapper &assetData) -> std::shared_ptr<panima::Animation> {
		auto anim = std::make_shared<panima::Animation>();
		if(anim->Load(assetData) == false)
			return nullptr;
		return anim;
	}))];
	animMod[cdAnim2];
}
