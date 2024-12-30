/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __ANIMATION_CHANNEL_CACHE_DATA_HPP__
#define __ANIMATION_CHANNEL_CACHE_DATA_HPP__

#include "pragma/networkdefinitions.h"
#include <array>

namespace pragma {
	class BaseEntityComponent;
	struct ComponentMemberInfo;
	struct DLLNETWORK AnimationChannelCacheData {
		enum class State : uint8_t {
			None = 0,
			Dirty = 1u,
			AlwaysDirty = Dirty << 1u,
		};
		pragma::BaseEntityComponent *component;
		const pragma::ComponentMemberInfo *memberInfo;
		State changed = State::Dirty;
		std::array<float, 16> data;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::AnimationChannelCacheData::State);

#endif
