// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.easing;

export import std.compat;

export namespace pragma::math {
	enum class EaseType : uint32_t { Back = 0u, Bounce, Circular, Cubic, Elastic, Exponential, Linear, Quadratic, Quartic, Quintic, Sine };
	DLLNETWORK float ease_in(float t, EaseType type = EaseType::Linear);
	DLLNETWORK float ease_out(float t, EaseType type = EaseType::Linear);
	DLLNETWORK float ease_in_out(float t, EaseType type = EaseType::Linear);
};
