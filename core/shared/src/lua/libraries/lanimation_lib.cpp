/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animation2.hpp"
#include "pragma/model/animation/animated_pose.hpp"
#include "pragma/model/animation/animation_channel.hpp"
#include "pragma/model/animation/animation_player.hpp"
#include "pragma/model/animation/animation_manager.hpp"
#include "pragma/model/animation/skeletal_animation.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include "pragma/model/model.h"
#include <luainterface.hpp>

namespace Lua::animation
{
	void register_library(Lua::Interface &lua);
};
void Lua::animation::register_library(Lua::Interface &lua)
{
	auto animMod = luabind::module(lua.GetState(),"animation");

	auto cdPose = luabind::class_<pragma::animation::AnimatedPose>("Pose");
	cdPose.def(luabind::tostring(luabind::self));
	cdPose.def(luabind::constructor<>());
	cdPose.def("SetTransformCount",static_cast<void(*)(lua_State*,pragma::animation::AnimatedPose&,uint32_t)>([](lua_State *l,pragma::animation::AnimatedPose &pose,uint32_t count) {
		pose.SetTransformCount(count);
	}));
	cdPose.def("SetBoneIndex",static_cast<void(*)(lua_State*,pragma::animation::AnimatedPose&,uint32_t,uint32_t)>([](lua_State *l,pragma::animation::AnimatedPose &pose,uint32_t idx,uint32_t boneIdx) {
		pose.SetBoneIndex(idx,boneIdx);
	}));
	cdPose.def("GetTransform",static_cast<umath::ScaledTransform*(*)(lua_State*,pragma::animation::AnimatedPose&,uint32_t)>([](lua_State *l,pragma::animation::AnimatedPose &pose,uint32_t boneIdx) -> umath::ScaledTransform* {
		return pose.GetTransform(boneIdx);
	}));
	cdPose.def("SetTransform",static_cast<void(*)(lua_State*,pragma::animation::AnimatedPose&,uint32_t,const umath::ScaledTransform&)>(
		[](lua_State *l,pragma::animation::AnimatedPose &pose,uint32_t boneIdx,const umath::ScaledTransform &transform) {
		pose.SetTransform(boneIdx,transform);
	}));
	cdPose.def("Clear",static_cast<void(*)(lua_State*,pragma::animation::AnimatedPose&)>(
		[](lua_State *l,pragma::animation::AnimatedPose &pose) {
		pose.Clear();
	}));
	cdPose.def("Lerp",static_cast<void(*)(lua_State*,pragma::animation::AnimatedPose&,const pragma::animation::AnimatedPose&,float)>(
		[](lua_State *l,pragma::animation::AnimatedPose &pose,const pragma::animation::AnimatedPose &other,float f) {
		pose.Lerp(other,f);
	}));
	cdPose.def("Localize",static_cast<void(*)(lua_State*,pragma::animation::AnimatedPose&,const Skeleton&)>(
		[](lua_State *l,pragma::animation::AnimatedPose &pose,const Skeleton &skeleton) {
		pose.Localize(skeleton);
	}));
	cdPose.def("Globalize",static_cast<void(*)(lua_State*,pragma::animation::AnimatedPose&,const Skeleton&)>(
		[](lua_State *l,pragma::animation::AnimatedPose &pose,const Skeleton &skeleton) {
		pose.Globalize(skeleton);
	}));
	cdPose.def("GetBoneTranslationTable",static_cast<luabind::tableT<uint32_t>(*)(lua_State*,pragma::animation::AnimatedPose&)>(
		[](lua_State *l,pragma::animation::AnimatedPose &pose) {
		return luabind::tableT<uint32_t>{Lua::vector_to_table(l,pose.GetBoneTranslationTable())};
	}));
	animMod[cdPose];

	auto cdChannel = luabind::class_<pragma::animation::AnimationChannel>("Channel");
	cdChannel.def(luabind::tostring(luabind::self));
	cdChannel.def("GetValueType",static_cast<udm::Type(*)(lua_State*,pragma::animation::AnimationChannel&)>([](lua_State *l,pragma::animation::AnimationChannel &channel) -> udm::Type {
		return channel.GetValueType();
	}));
	cdChannel.def("SetValueType",static_cast<void(*)(lua_State*,pragma::animation::AnimationChannel&,udm::Type)>([](lua_State *l,pragma::animation::AnimationChannel &channel,udm::Type type) {
		channel.SetValueType(type);
	}));
	cdChannel.def("GetInterpolation",static_cast<pragma::animation::AnimationChannelInterpolation(*)(lua_State*,pragma::animation::AnimationChannel&)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel) -> pragma::animation::AnimationChannelInterpolation {
		return channel.interpolation;
	}));
	cdChannel.def("SetInterpolation",static_cast<void(*)(lua_State*,pragma::animation::AnimationChannel&,pragma::animation::AnimationChannelInterpolation)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel,pragma::animation::AnimationChannelInterpolation interp) {
		channel.interpolation = interp;
	}));
	cdChannel.def("GetTargetPath",static_cast<util::Path(*)(lua_State*,pragma::animation::AnimationChannel&)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel) -> util::Path {
		return channel.targetPath;
	}));
	cdChannel.def("SetTargetPath",static_cast<void(*)(lua_State*,pragma::animation::AnimationChannel&,const std::string &path)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel,const std::string &path) {
		channel.targetPath = path;
	}));
	cdChannel.def("GetTimesArray",static_cast<udm::LinkedPropertyWrapper(*)(lua_State*,pragma::animation::AnimationChannel&)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel) -> udm::LinkedPropertyWrapper {
			return udm::LinkedPropertyWrapper{channel.GetTimesProperty()};
	}));
	cdChannel.def("GetValueArray",static_cast<udm::LinkedPropertyWrapper(*)(lua_State*,pragma::animation::AnimationChannel&)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel) -> udm::LinkedPropertyWrapper {
			return udm::LinkedPropertyWrapper{channel.GetValueProperty()};
	}));
	cdChannel.def("Save",static_cast<bool(*)(lua_State*,pragma::animation::AnimationChannel&,udm::LinkedPropertyWrapper&)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel,udm::LinkedPropertyWrapper &prop) -> bool {
		return channel.Save(prop);
	}));
	cdChannel.def("Load",static_cast<bool(*)(lua_State*,pragma::animation::AnimationChannel&,udm::LinkedPropertyWrapper&)>(
		[](lua_State *l,pragma::animation::AnimationChannel &channel,udm::LinkedPropertyWrapper &prop) -> bool {
		return channel.Load(prop);
	}));
	animMod[cdChannel];

	auto cdPlayer = luabind::class_<pragma::animation::AnimationPlayer>("Player");
	cdPlayer.def(luabind::tostring(luabind::self));
	cdPlayer.scope[luabind::def("create",static_cast<std::shared_ptr<pragma::animation::AnimationPlayer>(*)(lua_State*)>([](lua_State *l) {
		return pragma::animation::AnimationPlayer::Create();
	}))];
	cdPlayer.def("Advance",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&,float,bool)>([](lua_State *l,pragma::animation::AnimationPlayer &player,float dt,bool force) {
		player.Advance(dt,force);
	}));
	cdPlayer.def("Advance",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&,float)>([](lua_State *l,pragma::animation::AnimationPlayer &player,float dt) {
		player.Advance(dt);
	}));
	cdPlayer.def("GetDuration",static_cast<float(*)(lua_State*,pragma::animation::AnimationPlayer&)>([](lua_State *l,pragma::animation::AnimationPlayer &player) {
		return player.GetDuration();
	}));
	cdPlayer.def("GetRemainingDuration",static_cast<float(*)(lua_State*,pragma::animation::AnimationPlayer&)>([](lua_State *l,pragma::animation::AnimationPlayer &player) {
		return player.GetDuration();
	}));
	cdPlayer.def("GetCurrentTimeFraction",static_cast<float(*)(lua_State*,pragma::animation::AnimationPlayer&)>([](lua_State *l,pragma::animation::AnimationPlayer &player) {
		return player.GetCurrentTimeFraction();
	}));
	cdPlayer.def("GetCurrentTime",static_cast<float(*)(lua_State*,pragma::animation::AnimationPlayer&)>([](lua_State *l,pragma::animation::AnimationPlayer &player) {
		return player.GetCurrentTime();
	}));
	cdPlayer.def("GetPlaybackRate",static_cast<float(*)(lua_State*,pragma::animation::AnimationPlayer&)>([](lua_State *l,pragma::animation::AnimationPlayer &player) {
		return player.GetPlaybackRate();
	}));
	cdPlayer.def("SetPlaybackRate",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&,float)>([](lua_State *l,pragma::animation::AnimationPlayer &player,float playbackRate) {
		player.SetPlaybackRate(playbackRate);
	}));
	cdPlayer.def("SetCurrentTime",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&,float)>([](lua_State *l,pragma::animation::AnimationPlayer &player,float time) {
		player.SetCurrentTime(time);
	}));
	cdPlayer.def("SetCurrentTime",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&,float,bool)>([](lua_State *l,pragma::animation::AnimationPlayer &player,float time,bool force) {
		player.SetCurrentTime(time,force);
	}));
	cdPlayer.def("Reset",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&)>(
		[](lua_State *l,pragma::animation::AnimationPlayer &player) {
		player.Reset();
	}));
	cdPlayer.def("GetCurrentSlice",static_cast<pragma::animation::AnimationSlice*(*)(lua_State*,pragma::animation::AnimationPlayer&)>(
		[](lua_State *l,pragma::animation::AnimationPlayer &player) {
		return &player.GetCurrentSlice();
	}));
	cdPlayer.def("SetLooping",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&,bool)>(
		[](lua_State *l,pragma::animation::AnimationPlayer &player,bool looping) {
		player.SetLooping(looping);
	}));
	cdPlayer.def("IsLooping",static_cast<bool(*)(lua_State*,const pragma::animation::AnimationPlayer&)>(
		[](lua_State *l,const pragma::animation::AnimationPlayer &player) -> bool {
		return player.IsLooping();
	}));
	cdPlayer.def("SetAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationPlayer&,pragma::animation::Animation2&)>(
		[](lua_State *l,pragma::animation::AnimationPlayer &player,pragma::animation::Animation2 &anim) {
		player.SetAnimation(anim);
	}));
	animMod[cdPlayer];

	auto cdManager = luabind::class_<pragma::animation::AnimationManager>("Manager");
	cdManager.def(luabind::tostring(luabind::self));
	cdManager.scope[luabind::def("create",static_cast<std::shared_ptr<pragma::animation::AnimationManager>(*)(lua_State*,Model&)>([](lua_State *l,Model &mdl) {
		return pragma::animation::AnimationManager::Create(mdl);
	}))];
	cdManager.def("GetPreviousSlice",static_cast<pragma::animation::AnimationSlice*(*)(lua_State*,pragma::animation::AnimationManager&)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager) {
		return &manager.GetPreviousSlice();
	}));
	cdManager.def("GetCurrentAnimationId",static_cast<pragma::animation::AnimationId(*)(lua_State*,pragma::animation::AnimationManager&)>([](lua_State *l,pragma::animation::AnimationManager &manager) {
		return manager.GetCurrentAnimationId();
	}));
	cdManager.def("GetCurrentAnimation",static_cast<opt<std::shared_ptr<pragma::animation::Animation2>>(*)(lua_State*,pragma::animation::AnimationManager&)>([](lua_State *l,pragma::animation::AnimationManager &manager) -> luabind::optional<std::shared_ptr<pragma::animation::Animation2>> {
		auto *anim = manager.GetCurrentAnimation();
		return anim ? opt<std::shared_ptr<pragma::animation::Animation2>>{l,anim->shared_from_this()} : nil;
	}));
	cdManager.def("GetPlayer",static_cast<pragma::animation::PAnimationPlayer(*)(lua_State*,pragma::animation::AnimationManager&)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager) -> pragma::animation::PAnimationPlayer {
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
	cdManager.def("PlayAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationManager&,pragma::animation::AnimationId,pragma::FPlayAnim)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager,pragma::animation::AnimationId id,pragma::FPlayAnim flags) {
		manager.PlayAnimation(id,flags);
	}));
	cdManager.def("PlayAnimation",static_cast<void(*)(lua_State*,pragma::animation::AnimationManager&,pragma::animation::AnimationId)>(
		[](lua_State *l,pragma::animation::AnimationManager &manager,pragma::animation::AnimationId id) {
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

	auto cdSlice = luabind::class_<pragma::animation::AnimationSlice>("Slice");
	cdSlice.def(luabind::tostring(luabind::self));
	cdSlice.def("GetChannelValueCount",static_cast<uint32_t(*)(lua_State*,pragma::animation::AnimationSlice&)>([](lua_State *l,pragma::animation::AnimationSlice &slice) -> uint32_t {
		return slice.channelValues.size();
	}));
	cdSlice.def("GetChannelProperty",static_cast<luabind::optional<udm::PProperty>(*)(lua_State*,pragma::animation::AnimationSlice&,uint32_t)>([](lua_State *l,pragma::animation::AnimationSlice &slice,uint32_t idx) -> luabind::optional<udm::PProperty> {
		if(idx >= slice.channelValues.size())
			return nil;
		return {l,slice.channelValues[idx]};
	}));
	animMod[cdSlice];

	auto cdAnim2 = luabind::class_<pragma::animation::Animation2>("Animation2");
	cdAnim2.def(luabind::tostring(luabind::self));
	cdAnim2.scope[luabind::def("create",static_cast<std::shared_ptr<pragma::animation::Animation2>(*)(lua_State*)>([](lua_State *l) {
		return std::make_shared<pragma::animation::Animation2>();
	}))];
	cdAnim2.def("GetChannelCount",static_cast<uint32_t(*)(lua_State*,pragma::animation::Animation2&)>([](lua_State *l,pragma::animation::Animation2 &anim) {
		return anim.GetChannelCount();
	}));
	cdAnim2.def("GetAnimationSpeedFactor",static_cast<float(*)(lua_State*,pragma::animation::Animation2&)>([](lua_State *l,pragma::animation::Animation2 &anim) {
		return anim.GetAnimationSpeedFactor();
	}));
	cdAnim2.def("SetAnimationSpeedFactor",static_cast<void(*)(lua_State*,pragma::animation::Animation2&,float)>([](lua_State *l,pragma::animation::Animation2 &anim,float factor) {
		anim.SetAnimationSpeedFactor(factor);
	}));
	cdAnim2.def("GetDuration",static_cast<float(*)(lua_State*,pragma::animation::Animation2&)>([](lua_State *l,pragma::animation::Animation2 &anim) {
		return anim.GetDuration();
	}));
	cdAnim2.def("SetDuration",static_cast<void(*)(lua_State*,pragma::animation::Animation2&,float)>([](lua_State *l,pragma::animation::Animation2 &anim,float duration) {
		anim.SetDuration(duration);
	}));
	cdAnim2.def("AddChannel",static_cast<void(*)(lua_State*,pragma::animation::Animation2&,pragma::animation::AnimationChannel&)>([](lua_State *l,pragma::animation::Animation2 &anim,pragma::animation::AnimationChannel &channel) {
		anim.AddChannel(channel);
	}));
	cdAnim2.def("AddChannel",static_cast<opt<std::shared_ptr<pragma::animation::AnimationChannel>>(*)(lua_State*,pragma::animation::Animation2&,const util::Path&,udm::Type)>([](lua_State *l,pragma::animation::Animation2 &anim,const util::Path &path,udm::Type valueType) -> opt<std::shared_ptr<pragma::animation::AnimationChannel>> {
		auto *channel = anim.AddChannel(path,valueType);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	}));
	cdAnim2.def("AddChannel",static_cast<opt<std::shared_ptr<pragma::animation::AnimationChannel>>(*)(lua_State*,pragma::animation::Animation2&,const std::string&,udm::Type)>([](lua_State *l,pragma::animation::Animation2 &anim,const std::string &path,udm::Type valueType) -> opt<std::shared_ptr<pragma::animation::AnimationChannel>> {
		auto *channel = anim.AddChannel(path,valueType);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	}));
	cdAnim2.def("GetChannels",static_cast<tb<std::shared_ptr<pragma::animation::AnimationChannel>>(*)(lua_State*,pragma::animation::Animation2&)>([](lua_State *l,pragma::animation::Animation2 &anim) -> tb<std::shared_ptr<pragma::animation::AnimationChannel>> {
		return Lua::vector_to_table(l,anim.GetChannels());
	}));
	cdAnim2.def("FindChannel",static_cast<opt<std::shared_ptr<pragma::animation::AnimationChannel>>(*)(lua_State*,pragma::animation::Animation2&,const util::Path&)>([](lua_State *l,pragma::animation::Animation2 &anim,const util::Path &path) -> opt<std::shared_ptr<pragma::animation::AnimationChannel>> {
		auto *channel = anim.FindChannel(path);
		if(!channel)
			return nil;
		return {l,channel->shared_from_this()};
	}));
	animMod[cdAnim2];
}
