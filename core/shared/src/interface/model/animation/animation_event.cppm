// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:model.animation.animation_event;

import pragma.filesystem;

export {
	namespace pragma {
		struct DLLNETWORK AnimationEvent {
		  public:
			enum class DLLNETWORK Type : uint32_t { EmitSound, FootstepLeft, FootstepRight, Count, Invalid = std::numeric_limits<std::underlying_type_t<Type>>::max() };
		  public:
			Type eventID;
			std::vector<std::string> arguments;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::AnimationEvent::Type)

	extern DLLNETWORK std::unordered_map<int32_t, std::string> ANIMATION_EVENT_NAMES;

	namespace pragma {
		DLLNETWORK void register_engine_animation_events();
	};
};
