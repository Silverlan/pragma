// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
