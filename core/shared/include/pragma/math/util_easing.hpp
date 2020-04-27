/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_EASING_HPP__
#define __UTIL_EASING_HPP__

#include "pragma/networkdefinitions.h"

namespace umath
{
	enum class EaseType : uint32_t
	{
		Back = 0u,
		Bounce,
		Circular,
		Cubic,
		Elastic,
		Exponential,
		Linear,
		Quadratic,
		Quartic,
		Quintic,
		Sine
	};
	DLLNETWORK float ease_in(float t,EaseType type=EaseType::Linear);
	DLLNETWORK float ease_out(float t,EaseType type=EaseType::Linear);
	DLLNETWORK float ease_in_out(float t,EaseType type=EaseType::Linear);
};

#endif
