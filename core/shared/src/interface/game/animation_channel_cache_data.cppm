// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:game.animation_channel_cache_data;

import pragma.math;

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
			BaseEntityComponent *component;
			const ComponentMemberInfo *memberInfo;
			State changed = State::Dirty;
			std::array<float, 16> data;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::AnimationChannelCacheData::State);
};
