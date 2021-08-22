/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_ANIMATION_MANAGER_HPP__
#define __PRAGMA_ANIMATION_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include <panima/slice.hpp>
#include <udm.hpp>
#include <vector>
#include <memory>

class Model;
namespace pragma::animation
{
	struct DLLNETWORK AnimationPlayerCallbackInterface
	{
		std::function<bool(panima::AnimationId,FPlayAnim)> onPlayAnimation = nullptr;
		std::function<void()> onStopAnimation = nullptr;
		std::function<void(panima::AnimationId&,FPlayAnim&)> translateAnimation = nullptr;
	};
	using ChannelValueSubmitter = std::function<void(panima::Channel&,uint32_t&,double)>;
	class DLLNETWORK AnimationManager
		: public std::enable_shared_from_this<AnimationManager>
	{
	public:
		static std::shared_ptr<AnimationManager> Create(const Model &mdl);
		static std::shared_ptr<AnimationManager> Create(const AnimationManager &other);
		static std::shared_ptr<AnimationManager> Create(AnimationManager &&other);
		
		panima::AnimationId GetCurrentAnimationId() const {return m_currentAnimation;}
		panima::Animation *GetCurrentAnimation() const;
		const Model *GetModel() const;

		void PlayAnimation(panima::AnimationId animation,FPlayAnim flags=FPlayAnim::Default);
		void PlayAnimation(const std::string &animation,FPlayAnim flags=FPlayAnim::Default);
		void StopAnimation();

		panima::Slice &GetPreviousSlice() {return m_prevAnimSlice;}
		const panima::Slice &GetPreviousSlice() const {return const_cast<AnimationManager*>(this)->GetPreviousSlice();}

		AnimationManager &operator=(const AnimationManager &other);
		AnimationManager &operator=(AnimationManager &&other);

		panima::Player *operator->() {return m_player.get();}
		const panima::Player *operator->() const {return const_cast<AnimationManager*>(this)->operator->();}

		panima::Player &operator*() {return *m_player;}
		const panima::Player &operator*() const {return const_cast<AnimationManager*>(this)->operator*();}

		panima::Player &GetPlayer() {return *m_player;}
		const panima::Player &GetPlayer() const {return const_cast<AnimationManager*>(this)->GetPlayer();}

		std::vector<animation::ChannelValueSubmitter> &GetChannelValueSubmitters() {return m_channelValueSubmitters;}

		void SetCallbackInterface(const AnimationPlayerCallbackInterface &i) {m_callbackInterface = i;}
	private:
		AnimationManager(const Model &mdl);
		AnimationManager(const AnimationManager &other);
		AnimationManager(AnimationManager &&other);
		static void ApplySliceInterpolation(const panima::Slice &src,panima::Slice &dst,float f);
		panima::PPlayer m_player = nullptr;
		std::weak_ptr<const Model> m_model {};
		panima::AnimationId m_currentAnimation = std::numeric_limits<panima::AnimationId>::max();
		FPlayAnim m_currentFlags = FPlayAnim::None;
		std::vector<animation::ChannelValueSubmitter> m_channelValueSubmitters {};
		
		panima::Slice m_prevAnimSlice;
		AnimationPlayerCallbackInterface m_callbackInterface {};
	};
	using PAnimationManager = std::shared_ptr<AnimationManager>;
};

DLLNETWORK std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationManager &o);

#endif
