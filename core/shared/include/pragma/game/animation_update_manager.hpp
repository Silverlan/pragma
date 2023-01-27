/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ANIMATION_UPDATE_MANAGER_HPP__
#define __ANIMATION_UPDATE_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/util/util_thread_pool.hpp"

class Game;
namespace pragma
{
	struct AnimationUpdateManager
	{
		AnimationUpdateManager(Game &game);

		void UpdateAnimations(double dt);
	private:
		void UpdateEntityAnimationDrivers(double dt);
		void UpdateConstraints(double dt);

		Game &game;
		pragma::ComponentId m_animatedComponentId = std::numeric_limits<pragma::ComponentId>::max();
		pragma::ComponentId m_panimaComponentId = std::numeric_limits<pragma::ComponentId>::max();
		pragma::ComponentId m_animationDriverComponentId = std::numeric_limits<pragma::ComponentId>::max();
		pragma::ThreadPool m_threadPool;
	};
};

#endif
