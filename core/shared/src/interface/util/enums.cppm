// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:util.enums;

export import std.compat;

export {
	namespace pragma::util {
		enum class DurationType : uint32_t { NanoSeconds = 0u, MicroSeconds, MilliSeconds, Seconds, Minutes, Hours };
		enum class FogType : uint8_t { Linear = 0, Exponential = 1, Exponential2 = 2 };
	};
};
