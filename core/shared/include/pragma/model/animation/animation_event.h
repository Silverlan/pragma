/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __ANIMATION_EVENT_H__
#define __ANIMATION_EVENT_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

struct DLLNETWORK AnimationEvent
{
public:
	enum class DLLNETWORK Type : uint32_t
	{
		EmitSound,
		FootstepLeft,
		FootstepRight,
		Count,
		Invalid = std::numeric_limits<std::underlying_type_t<Type>>::max()
	};
public:
	Type eventID;
	std::vector<std::string> arguments;
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(AnimationEvent::Type)

extern DLLNETWORK std::unordered_map<int32_t,std::string> ANIMATION_EVENT_NAMES;

namespace pragma
{
	DLLNETWORK void register_engine_animation_events();
};

#endif
