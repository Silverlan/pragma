// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ANIMATION_UPDATE_MANAGER_HPP__
#define __ANIMATION_UPDATE_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/util/util_thread_pool.hpp"
#include "pragma/game/global_animation_channel_queue_processor.hpp"

class Game;
namespace pragma {
	class PanimaComponent;
	class BaseAnimatedComponent;
	struct DLLNETWORK AnimationUpdateManager {
		struct DLLNETWORK AnimatedEntity {
			BaseEntity *entity = nullptr;
			BaseAnimatedComponent *animatedC = nullptr;
			PanimaComponent *panimaC = nullptr;
		};

		AnimationUpdateManager(Game &game);

		void UpdateEntityState(BaseEntity &ent);
		const std::vector<AnimatedEntity> &GetAnimatedEntities() const;

		void UpdateAnimations(double dt);
	  private:
		void UpdateEntityAnimationDrivers(double dt);
		void UpdateConstraints(double dt);

		Game &game;
		pragma::ComponentId m_animatedComponentId = std::numeric_limits<pragma::ComponentId>::max();
		pragma::ComponentId m_panimaComponentId = std::numeric_limits<pragma::ComponentId>::max();
		pragma::ComponentId m_animationDriverComponentId = std::numeric_limits<pragma::ComponentId>::max();
		pragma::ComponentId m_constraintManagerComponentId = std::numeric_limits<pragma::ComponentId>::max();
		std::vector<AnimatedEntity> m_animatedEntities;
		std::vector<BaseAnimatedComponent *> m_postAnimListenerQueue;
		GlobalAnimationChannelQueueProcessor m_channelQueueProcessor;
	};
};

#endif
