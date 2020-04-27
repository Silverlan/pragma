/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_DURATION_TYPE_HPP__
#define __UTIL_DURATION_TYPE_HPP__

#include <cinttypes>

namespace util
{
	enum class DurationType : uint32_t
	{
		NanoSeconds = 0u,
		MicroSeconds,
		MilliSeconds,
		Seconds,
		Minutes,
		Hours
	};
};

#endif
