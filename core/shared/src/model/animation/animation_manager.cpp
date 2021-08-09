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
#include "pragma/model/animation/animation_manager.hpp"

std::shared_ptr<pragma::animation::AnimationManager> pragma::animation::AnimationManager::Create(const Model &mdl)
{
	return std::shared_ptr<AnimationManager>{new AnimationManager{mdl}};
}
std::shared_ptr<pragma::animation::AnimationManager> pragma::animation::AnimationManager::Create(const AnimationManager &other)
{
	return std::shared_ptr<AnimationManager>{new AnimationManager{other}};
}
std::shared_ptr<pragma::animation::AnimationManager> pragma::animation::AnimationManager::Create(AnimationManager &&other)
{
	return std::shared_ptr<AnimationManager>{new AnimationManager{std::move(other)}};
}
pragma::animation::AnimationManager::AnimationManager(const Model &mdl)
	: m_player{AnimationPlayer::Create()},m_model{mdl.shared_from_this()}
{}
pragma::animation::AnimationManager::AnimationManager(const AnimationManager &other)
	: m_player{AnimationPlayer::Create(*other.m_player)},m_model{other.m_model},m_currentAnimation{other.m_currentAnimation},
	m_prevAnimSlice{other.m_prevAnimSlice}
{
	static_assert(sizeof(*this) == 272,"Update this implementation when class has changed!");
}
pragma::animation::AnimationManager::AnimationManager(AnimationManager &&other)
	: m_player{AnimationPlayer::Create(*other.m_player)},m_model{other.m_model},m_currentAnimation{other.m_currentAnimation},
	m_prevAnimSlice{std::move(other.m_prevAnimSlice)}
{
	static_assert(sizeof(*this) == 272,"Update this implementation when class has changed!");
}
pragma::animation::AnimationManager &pragma::animation::AnimationManager::operator=(const AnimationManager &other)
{
	m_player = AnimationPlayer::Create(*other.m_player);
	m_model = other.m_model;
	m_currentAnimation = other.m_currentAnimation;

	m_prevAnimSlice = other.m_prevAnimSlice;
	static_assert(sizeof(*this) == 272,"Update this implementation when class has changed!");
	return *this;
}
pragma::animation::AnimationManager &pragma::animation::AnimationManager::operator=(AnimationManager &&other)
{
	m_player = AnimationPlayer::Create(*other.m_player);
	m_model = other.m_model;
	m_currentAnimation = other.m_currentAnimation;

	m_prevAnimSlice = std::move(other.m_prevAnimSlice);
	static_assert(sizeof(*this) == 272,"Update this implementation when class has changed!");
	return *this;
}

pragma::animation::Animation2 *pragma::animation::AnimationManager::GetCurrentAnimation() const {return nullptr;} // TODO
const Model *pragma::animation::AnimationManager::GetModel() const {return m_model.lock().get();}

void pragma::animation::AnimationManager::PlayAnimation(const std::string &animation,FPlayAnim flags)
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

void pragma::animation::AnimationManager::PlayAnimation(AnimationId animation,FPlayAnim flags)
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

	if(animation == m_currentAnimation && (*this)->GetCurrentTime() == 0.f && m_currentFlags == flags)
		return; // No change
	if(m_callbackInterface.onPlayAnimation && m_callbackInterface.onPlayAnimation(animation,flags) == false)
		return;
	m_currentAnimation = animation;
	(*this)->Reset();
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
void pragma::animation::AnimationManager::StopAnimation()
{
	if(m_currentAnimation == INVALID_ANIMATION)
		return;
	if(m_callbackInterface.onStopAnimation)
		m_callbackInterface.onStopAnimation();
	m_currentAnimation = INVALID_ANIMATION;
	(*this)->Reset();
	m_currentFlags = FPlayAnim::None;
}
void pragma::animation::AnimationManager::ApplySliceInterpolation(const AnimationSlice &src,AnimationSlice &dst,float f)
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

std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationManager &o)
{
	out<<"AnimationManager";
	out<<"[Player:"<<*o<<"]";
	return out;
}
