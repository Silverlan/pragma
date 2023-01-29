/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ANIMATION_CHANNEL_DESC_HPP__
#define __ANIMATION_CHANNEL_DESC_HPP__

#include "pragma/networkdefinitions.h"
#include <memory>

namespace pragma {
	class AnimationChannel;
	struct DLLNETWORK AnimChannelDesc {
		std::shared_ptr<AnimationChannel> channel = nullptr;
		uint32_t lastTimestampIndex = 0u;
	};
};

#endif
