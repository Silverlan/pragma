/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/model.h"
#include "pragma/model/animation/animation.hpp"
#include "pragma/model/animation/animation_channel.hpp"
#include "pragma/model/animation/animation_player.hpp"


std::shared_ptr<pragma::animation::AnimationPlayer> pragma::animation::AnimationPlayer::Create(const Model &mdl)
{
	return std::shared_ptr<AnimationPlayer>{new AnimationPlayer{mdl}};
}
std::shared_ptr<pragma::animation::AnimationPlayer> pragma::animation::AnimationPlayer::Create(const AnimationPlayer &other)
{
	return std::shared_ptr<AnimationPlayer>{new AnimationPlayer{other}};
}
std::shared_ptr<pragma::animation::AnimationPlayer> pragma::animation::AnimationPlayer::Create(AnimationPlayer &&other)
{
	return std::shared_ptr<AnimationPlayer>{new AnimationPlayer{std::move(other)}};
}
pragma::animation::AnimationPlayer::AnimationPlayer(const Model &mdl)
	: m_model{mdl.shared_from_this()}
{}
pragma::animation::AnimationPlayer::AnimationPlayer(const AnimationPlayer &other)
	: m_model{other.m_model},m_playbackRate{other.m_playbackRate},m_currentAnimation{other.m_currentAnimation},
	m_currentTime{other.m_currentTime},m_currentFlags{other.m_currentFlags},m_lastChannelTimestampIndices{other.m_lastChannelTimestampIndices},
	m_currentSlice{other.m_currentSlice},m_prevAnimSlice{other.m_prevAnimSlice}
{
	static_assert(sizeof(*this) == 312,"Update this implementation when class has changed!");
}
pragma::animation::AnimationPlayer::AnimationPlayer(AnimationPlayer &&other)
	: m_model{other.m_model},m_playbackRate{other.m_playbackRate},m_currentAnimation{other.m_currentAnimation},
	m_currentTime{other.m_currentTime},m_currentFlags{other.m_currentFlags},m_lastChannelTimestampIndices{std::move(other.m_lastChannelTimestampIndices)},
	m_currentSlice{std::move(other.m_currentSlice)},m_prevAnimSlice{std::move(other.m_prevAnimSlice)}
{
	static_assert(sizeof(*this) == 312,"Update this implementation when class has changed!");
}
pragma::animation::AnimationPlayer &pragma::animation::AnimationPlayer::operator=(const AnimationPlayer &other)
{
	m_model = other.m_model;
	m_playbackRate = other.m_playbackRate;
	m_currentAnimation = other.m_currentAnimation;
	m_currentTime = other.m_currentTime;
	m_currentFlags = other.m_currentFlags;

	m_lastChannelTimestampIndices = other.m_lastChannelTimestampIndices;
	m_currentSlice = other.m_currentSlice;
	m_prevAnimSlice = other.m_prevAnimSlice;
	static_assert(sizeof(*this) == 312,"Update this implementation when class has changed!");
	return *this;
}
pragma::animation::AnimationPlayer &pragma::animation::AnimationPlayer::operator=(AnimationPlayer &&other)
{
	m_model = other.m_model;
	m_playbackRate = other.m_playbackRate;
	m_currentAnimation = other.m_currentAnimation;
	m_currentTime = other.m_currentTime;
	m_currentFlags = other.m_currentFlags;

	m_lastChannelTimestampIndices = std::move(other.m_lastChannelTimestampIndices);
	m_currentSlice = std::move(other.m_currentSlice);
	m_prevAnimSlice = std::move(other.m_prevAnimSlice);
	static_assert(sizeof(*this) == 312,"Update this implementation when class has changed!");
	return *this;
}
float pragma::animation::AnimationPlayer::GetDuration() const
{
	auto mdl = m_model.lock();
	auto anim = mdl ? mdl->GetAnimation(m_currentAnimation) : nullptr;
	return anim ? anim->GetDuration() : 0.f;
}
float pragma::animation::AnimationPlayer::GetRemainingAnimationDuration() const {return GetDuration() -GetCurrentTime();}
float pragma::animation::AnimationPlayer::GetCurrentTimeFraction() const
{
	auto t = GetCurrentTime();
	auto dur = GetDuration();
	return (dur > 0.f) ? (t /dur) : 0.f;
}
void pragma::animation::AnimationPlayer::SetCurrentTime(float t,bool forceUpdate)
{
	if(t == m_currentTime && !forceUpdate)
		return;
	m_currentTime = t;
	Advance(0.f,true);
}
void pragma::animation::AnimationPlayer::Advance(float dt,bool forceUpdate)
{
	if(m_currentAnimation == INVALID_ANIMATION || m_model.expired())
		return;
	auto mdl = m_model.lock();
	auto anim = mdl->GetAnimation(m_currentAnimation);
	if(!anim)
		return;
	dt *= m_playbackRate;
	auto newTime = m_currentTime;
	newTime += dt;
	auto dur = anim->GetDuration();
	if(newTime > dur)
	{
		if(umath::is_flag_set(m_currentFlags,FPlayAnim::Loop) && dur > 0.f)
		{
			auto d = fmodf(newTime,dur);
			newTime = d;
		}
		else
			newTime = dur;
	}
	if(newTime == m_currentTime && !forceUpdate)
		return;
	m_currentTime = newTime;
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto &channels = anim->GetChannels();
	auto numChannels = umath::min(channels.size(),m_currentSlice.channelValues.size());
	auto vs = [this,newTime](auto tag,pragma::animation::AnimationChannel &channel,udm::Property &sliceData,uint32_t &inOutPivotTimeIndex) {
		using T = decltype(tag)::type;
		if constexpr(
			!std::is_same_v<T,udm::Vector2i> && !std::is_same_v<T,udm::Vector3i> && !std::is_same_v<T,udm::Vector4i> && !std::is_same_v<T,udm::Srgba> && !std::is_same_v<T,udm::HdrColor> &&
			!std::is_same_v<T,udm::Transform> && !std::is_same_v<T,udm::ScaledTransform> && !std::is_same_v<T,udm::Nil>
		)
			sliceData = channel.GetInterpolatedValue<T>(newTime,inOutPivotTimeIndex);
	};
	for(auto i=decltype(numChannels){0u};i<numChannels;++i)
	{
		auto &channel = channels[i];
		auto &sliceData = m_currentSlice.channelValues[i];
		auto &lastChannelTimestampIndex = m_lastChannelTimestampIndices[i];
		if(udm::is_numeric_type(channel->valueType))
			std::visit([&vs,&channel,&sliceData,&lastChannelTimestampIndex](auto tag) {vs(tag,*channel,sliceData,lastChannelTimestampIndex);},udm::get_numeric_tag(channel->valueType));
		else if(udm::is_generic_type(channel->valueType))
			std::visit([&vs,&channel,&sliceData,&lastChannelTimestampIndex](auto tag) {vs(tag,*channel,sliceData,lastChannelTimestampIndex);},udm::get_generic_tag(channel->valueType));
	}
#endif
	// TODO
	// ApplySliceInterpolation(m_prevAnimSlice,m_currentSlice,fadeFactor);
}

void pragma::animation::AnimationPlayer::PlayAnimation(const std::string &animation,FPlayAnim flags)
{
	auto mdl = m_model.lock();
	if(!mdl)
	{
		StopAnimation();
		return;
	}
	auto id = mdl->LookupAnimation(animation);
	if(id == -1)
	{
		StopAnimation();
		return;
	}
	return PlayAnimation(id,flags);
}

void pragma::animation::AnimationPlayer::PlayAnimation(AnimationId animation,FPlayAnim flags)
{
	if(m_model.expired())
	{
		StopAnimation();
		return;
	}
	if(m_callbackInterface.translateAnimation && animation != INVALID_ANIMATION)
		m_callbackInterface.translateAnimation(animation,flags);
	if(animation == INVALID_ANIMATION)
	{
		StopAnimation();
		return;
	}
	auto mdl = m_model.lock();
	auto anim = mdl->GetAnimation(animation);
	if(!anim)
	{
		StopAnimation();
		return;
	}
	if(animation == m_currentAnimation && (flags &FPlayAnim::Reset) == FPlayAnim::None)
	{
		if(anim != NULL && anim->HasFlag(FAnim::Loop))
			return;
	}

	if(animation == m_currentAnimation && m_currentTime == 0.f && m_currentFlags == flags)
		return; // No change
	if(m_callbackInterface.onPlayAnimation && m_callbackInterface.onPlayAnimation(animation,flags) == false)
		return;
	m_currentAnimation = animation;
	m_currentTime = 0;
	m_currentFlags = flags;
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto &channels = anim->GetChannels();
	m_currentSlice.channelValues.resize(channels.size());
	m_lastChannelTimestampIndices.resize(channels.size(),0u);
	for(auto i=decltype(channels.size()){0u};i<channels.size();++i)
	{
		auto &channel = channels[i];
		auto &sliceValue = m_currentSlice.channelValues[i];
		sliceValue = udm::Property::Create(channel->valueType);
	}
	SetCurrentTime(0.f,true);
#endif
}
void pragma::animation::AnimationPlayer::StopAnimation()
{
	if(m_currentAnimation == INVALID_ANIMATION)
		return;
	if(m_callbackInterface.onStopAnimation)
		m_callbackInterface.onStopAnimation();
	m_currentAnimation = INVALID_ANIMATION;
	m_currentTime = 0.f;
	m_currentFlags = FPlayAnim::None;
	m_lastChannelTimestampIndices.clear();
	m_currentSlice.channelValues.clear();
}
void pragma::animation::AnimationPlayer::ApplySliceInterpolation(const AnimationSlice &src,AnimationSlice &dst,float f)
{
	// TODO
	/*if(f == 1.f)
		return;
	auto n = src.channelValues.size();
	for(auto i=decltype(n){0u};i<n;++i)
	{
		//auto &channel = m_channels[i];
		pragma::AnimationChannel &channel ;// TODO
		// TODO: How to translate channels?
		auto &srcVal = src.channelValues[i];
		auto &dstVal = dst.channelValues[i];
		// TODO
		auto interp = channel.GetInterpolationFunction<Vector3>();
		dstVal = interp(srcVal.GetValue<Vector3>(),dstVal.GetValue<Vector3>(),f);
	}*/
}

std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationPlayer &o)
{
	out<<"AnimationPlayer";
	out<<"[Time:"<<o.GetCurrentTime()<<"/"<<o.GetDuration()<<"]";
	out<<"[PlaybackRate:"<<o.GetPlaybackRate()<<"]";
	out<<"[AnimId:"<<o.GetCurrentAnimationId()<<"]";
	return out;
}

std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationSlice &o)
{
	out<<"AnimationSlice";
	out<<"[Values:"<<o.channelValues.size()<<"]";
	return out;
}
