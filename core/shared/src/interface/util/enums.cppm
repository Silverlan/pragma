// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>

export module pragma.shared:util.enums;

export {
	namespace util {
		enum class DurationType : uint32_t { NanoSeconds = 0u, MicroSeconds, MilliSeconds, Seconds, Minutes, Hours };
	};
};
