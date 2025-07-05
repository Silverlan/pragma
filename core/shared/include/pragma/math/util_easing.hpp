// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_EASING_HPP__
#define __UTIL_EASING_HPP__

#include "pragma/networkdefinitions.h"

namespace umath {
	enum class EaseType : uint32_t { Back = 0u, Bounce, Circular, Cubic, Elastic, Exponential, Linear, Quadratic, Quartic, Quintic, Sine };
	DLLNETWORK float ease_in(float t, EaseType type = EaseType::Linear);
	DLLNETWORK float ease_out(float t, EaseType type = EaseType::Linear);
	DLLNETWORK float ease_in_out(float t, EaseType type = EaseType::Linear);
};

#endif
