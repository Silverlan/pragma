// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_DURATION_TYPE_HPP__
#define __UTIL_DURATION_TYPE_HPP__

#include <cinttypes>

namespace util {
	enum class DurationType : uint32_t { NanoSeconds = 0u, MicroSeconds, MilliSeconds, Seconds, Minutes, Hours };
};

#endif
