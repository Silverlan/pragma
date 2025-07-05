// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ANIMATION_EVENT_H__
#define __ANIMATION_EVENT_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

struct DLLNETWORK AnimationEvent {
  public:
	enum class DLLNETWORK Type : uint32_t { EmitSound, FootstepLeft, FootstepRight, Count, Invalid = std::numeric_limits<std::underlying_type_t<Type>>::max() };
  public:
	Type eventID;
	std::vector<std::string> arguments;
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(AnimationEvent::Type)

extern DLLNETWORK std::unordered_map<int32_t, std::string> ANIMATION_EVENT_NAMES;

namespace pragma {
	DLLNETWORK void register_engine_animation_events();
};

#endif
