/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_ANIMATION_PLAYER_HPP__
#define __PRAGMA_ANIMATION_PLAYER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include <udm.hpp>
#include <vector>
#include <memory>

#undef GetCurrentTime
// #define PRAGMA_ENABLE_ANIMATION_SYSTEM_2

class Model;
namespace pragma::animation
{
	struct DLLNETWORK AnimationSlice
	{
		AnimationSlice()=default;
		AnimationSlice(const AnimationSlice&)=default;
		AnimationSlice(AnimationSlice &&other)=default;
		AnimationSlice &operator=(const AnimationSlice&)=default;
		AnimationSlice &operator=(AnimationSlice &&)=default;
		std::vector<udm::Property> channelValues;
	};
	struct DLLNETWORK AnimationPlayerCallbackInterface
	{
		std::function<bool(AnimationId,FPlayAnim)> onPlayAnimation = nullptr;
		std::function<void()> onStopAnimation = nullptr;
		std::function<void(AnimationId&,FPlayAnim&)> translateAnimation = nullptr;
	};
	class DLLNETWORK AnimationPlayer
		: public std::enable_shared_from_this<AnimationPlayer>
	{
	public:
		static std::shared_ptr<AnimationPlayer> Create(const Model &mdl);
		static std::shared_ptr<AnimationPlayer> Create(const AnimationPlayer &other);
		static std::shared_ptr<AnimationPlayer> Create(AnimationPlayer &&other);
		void Advance(float dt,bool force=false);

		AnimationId GetCurrentAnimationId() const {return m_currentAnimation;}
		Animation *GetCurrentAnimation() const;
		Model *GetModel() const;
		float GetDuration() const;
		float GetRemainingAnimationDuration() const;
		float GetCurrentTimeFraction() const;
		float GetCurrentTime() const {return m_currentTime;}
		float GetPlaybackRate() const {return m_playbackRate;}
		void SetPlaybackRate(float playbackRate) {m_playbackRate = playbackRate;}
		void SetCurrentTime(float t,bool forceUpdate=false);

		void PlayAnimation(AnimationId animation,FPlayAnim flags=FPlayAnim::Default);
		void PlayAnimation(const std::string &animation,FPlayAnim flags=FPlayAnim::Default);
		void StopAnimation();

		AnimationSlice &GetCurrentSlice() {return m_currentSlice;}
		const AnimationSlice &GetCurrentSlice() const {return const_cast<AnimationPlayer*>(this)->GetCurrentSlice();}
		AnimationSlice &GetPreviousSlice() {return m_prevAnimSlice;}
		const AnimationSlice &GetPreviousSlice() const {return const_cast<AnimationPlayer*>(this)->GetPreviousSlice();}

		AnimationPlayer &operator=(const AnimationPlayer &other);
		AnimationPlayer &operator=(AnimationPlayer &&other);

		void SetCallbackInterface(const AnimationPlayerCallbackInterface &i) {m_callbackInterface = i;}
	private:
		AnimationPlayer(const Model &mdl);
		AnimationPlayer(const AnimationPlayer &other);
		AnimationPlayer(AnimationPlayer &&other);
		static void ApplySliceInterpolation(const AnimationSlice &src,AnimationSlice &dst,float f);
		std::weak_ptr<const Model> m_model {};
		float m_playbackRate = 1.f;
		AnimationId m_currentAnimation = std::numeric_limits<AnimationId>::max();
		float m_currentTime = 0.f;
		FPlayAnim m_currentFlags = FPlayAnim::None;

		AnimationPlayerCallbackInterface m_callbackInterface {};
		std::vector<uint32_t> m_lastChannelTimestampIndices;
		AnimationSlice m_currentSlice;
		AnimationSlice m_prevAnimSlice;
	};
	using PAnimationPlayer = std::shared_ptr<AnimationPlayer>;
};

#endif
