// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <memory>

export module pragma.shared:model.animation.anim_channel_desc;

export namespace pragma {
	struct DLLNETWORK AnimChannelDesc {
		std::shared_ptr<AnimationChannel> channel = nullptr;
		uint32_t lastTimestampIndex = 0u;
	};
};
