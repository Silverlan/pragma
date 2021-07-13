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
#include "pragma/model/animation/animation_player.hpp"
#include <udm.hpp>
#include <vector>
#include <memory>

class Model;
namespace pragma::animation
{
	struct DLLNETWORK AnimationPlayerCallbackInterface
	{
		std::function<bool(AnimationId,FPlayAnim)> onPlayAnimation = nullptr;
		std::function<void()> onStopAnimation = nullptr;
		std::function<void(AnimationId&,FPlayAnim&)> translateAnimation = nullptr;
	};
	class AnimationPlayer;
	using PAnimationPlayer = std::shared_ptr<AnimationPlayer>;
	class DLLNETWORK AnimationManager
		: public std::enable_shared_from_this<AnimationManager>
	{
	public:
		static std::shared_ptr<AnimationManager> Create(const Model &mdl);
		static std::shared_ptr<AnimationManager> Create(const AnimationManager &other);
		static std::shared_ptr<AnimationManager> Create(AnimationManager &&other);
		
		AnimationId GetCurrentAnimationId() const {return m_currentAnimation;}
		Animation2 *GetCurrentAnimation() const;
		const Model *GetModel() const;

		void PlayAnimation(AnimationId animation,FPlayAnim flags=FPlayAnim::Default);
		void PlayAnimation(const std::string &animation,FPlayAnim flags=FPlayAnim::Default);
		void StopAnimation();

		AnimationSlice &GetPreviousSlice() {return m_prevAnimSlice;}
		const AnimationSlice &GetPreviousSlice() const {return const_cast<AnimationManager*>(this)->GetPreviousSlice();}

		AnimationManager &operator=(const AnimationManager &other);
		AnimationManager &operator=(AnimationManager &&other);

		AnimationPlayer *operator->() {return m_player.get();}
		const AnimationPlayer *operator->() const {return const_cast<AnimationManager*>(this)->operator->();}

		AnimationPlayer &operator*() {return *m_player;}
		const AnimationPlayer &operator*() const {return const_cast<AnimationManager*>(this)->operator*();}

		AnimationPlayer &GetPlayer() {return *m_player;}
		const AnimationPlayer &GetPlayer() const {return const_cast<AnimationManager*>(this)->GetPlayer();}

		void SetCallbackInterface(const AnimationPlayerCallbackInterface &i) {m_callbackInterface = i;}
	private:
		AnimationManager(const Model &mdl);
		AnimationManager(const AnimationManager &other);
		AnimationManager(AnimationManager &&other);
		static void ApplySliceInterpolation(const AnimationSlice &src,AnimationSlice &dst,float f);
		PAnimationPlayer m_player = nullptr;
		std::weak_ptr<const Model> m_model {};
		AnimationId m_currentAnimation = std::numeric_limits<AnimationId>::max();
		FPlayAnim m_currentFlags = FPlayAnim::None;
		
		AnimationSlice m_prevAnimSlice;
		AnimationPlayerCallbackInterface m_callbackInterface {};
	};
	using PAnimationManager = std::shared_ptr<AnimationManager>;
};

DLLNETWORK std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationManager &o);

#endif
