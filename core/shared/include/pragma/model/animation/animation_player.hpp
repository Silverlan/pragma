/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_ANIMATION_PLAYER_HPP__
#define __PRAGMA_ANIMATION_PLAYER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include <udm.hpp>
#include <vector>
#include <memory>

#undef GetCurrentTime
// #define PRAGMA_ENABLE_ANIMATION_SYSTEM_2

class Model;
namespace pragma::animation
{
	class Animation2;
	struct DLLNETWORK AnimationSlice
	{
		AnimationSlice()=default;
		AnimationSlice(const AnimationSlice&)=default;
		AnimationSlice(AnimationSlice &&other)=default;
		AnimationSlice &operator=(const AnimationSlice&)=default;
		AnimationSlice &operator=(AnimationSlice &&)=default;
		std::vector<udm::PProperty> channelValues;
	};
	class DLLNETWORK AnimationPlayer
		: public std::enable_shared_from_this<AnimationPlayer>
	{
	public:
		static std::shared_ptr<AnimationPlayer> Create();
		static std::shared_ptr<AnimationPlayer> Create(const AnimationPlayer &other);
		static std::shared_ptr<AnimationPlayer> Create(AnimationPlayer &&other);
		void Advance(float dt,bool force=false);

		float GetDuration() const;
		float GetRemainingAnimationDuration() const;
		float GetCurrentTimeFraction() const;
		float GetCurrentTime() const {return m_currentTime;}
		void SetCurrentTimeFraction(float t,bool forceUpdate);
		float GetPlaybackRate() const {return m_playbackRate;}
		void SetPlaybackRate(float playbackRate) {m_playbackRate = playbackRate;}
		void SetCurrentTime(float t,bool forceUpdate=false);
		
		AnimationSlice &GetCurrentSlice() {return m_currentSlice;}
		const AnimationSlice &GetCurrentSlice() const {return const_cast<AnimationPlayer*>(this)->GetCurrentSlice();}

		void SetLooping(bool looping) {m_looping = looping;}
		bool IsLooping() const {return m_looping;}

		void SetAnimation(const Animation2 &animation);
		void Reset();

		const Animation2 *GetAnimation() const {return m_animation.get();}

		AnimationPlayer &operator=(const AnimationPlayer &other);
		AnimationPlayer &operator=(AnimationPlayer &&other);
	private:
		AnimationPlayer();
		AnimationPlayer(const AnimationPlayer &other);
		AnimationPlayer(AnimationPlayer &&other);
		static void ApplySliceInterpolation(const AnimationSlice &src,AnimationSlice &dst,float f);
		std::shared_ptr<const Animation2> m_animation = nullptr;
		AnimationSlice m_currentSlice;
		float m_playbackRate = 1.f;
		float m_currentTime = 0.f;
		bool m_looping = false;

		std::vector<uint32_t> m_lastChannelTimestampIndices;
	};
	using PAnimationPlayer = std::shared_ptr<AnimationPlayer>;
};

DLLNETWORK std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationPlayer &o);
DLLNETWORK std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationSlice &o);

#endif
