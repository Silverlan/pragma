// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.random;

export import std.compat;

export namespace pragma::math {
	DLLNETWORK void to_random_float(const std::string &s, float &outF0, float &outF1);
	DLLNETWORK std::uniform_real_distribution<float> to_random_float(const std::string &s);
	DLLNETWORK void to_random_int(const std::string &s, int32_t &outI0, int32_t &outI1);
	DLLNETWORK std::uniform_int_distribution<int32_t> to_random_int(const std::string &s);
};
