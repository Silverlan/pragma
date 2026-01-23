// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:game.animation_update_manager;

export import :game.global_animation_channel_queue_processor;
export import :model.enums;
export import :types;

export {
	namespace pragma {
		struct DLLNETWORK AnimationUpdateManager {
			struct DLLNETWORK AnimatedEntity {
				ecs::BaseEntity *entity = nullptr;
				BaseAnimatedComponent *animatedC = nullptr;
				PanimaComponent *panimaC = nullptr;
			};

			AnimationUpdateManager(Game &game);

			void UpdateEntityState(ecs::BaseEntity &ent);
			const std::vector<AnimatedEntity> &GetAnimatedEntities() const;

			void UpdateAnimations(double dt);
		  private:
			void UpdateEntityAnimationDrivers(double dt);
			void UpdateConstraints(double dt);

			Game &game;
			ComponentId m_animatedComponentId = std::numeric_limits<ComponentId>::max();
			ComponentId m_panimaComponentId = std::numeric_limits<ComponentId>::max();
			ComponentId m_animationDriverComponentId = std::numeric_limits<ComponentId>::max();
			ComponentId m_constraintManagerComponentId = std::numeric_limits<ComponentId>::max();
			std::vector<AnimatedEntity> m_animatedEntities;
			std::vector<BaseAnimatedComponent *> m_postAnimListenerQueue;
			GlobalAnimationChannelQueueProcessor m_channelQueueProcessor;
		};
	};
};
