// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <array>
#include <mathutil/umath.h>

export module pragma.shared:game.animation_channel_cache_data;

export {
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
};
