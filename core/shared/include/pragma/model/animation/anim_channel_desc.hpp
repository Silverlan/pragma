// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
