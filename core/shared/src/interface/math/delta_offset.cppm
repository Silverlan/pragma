// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.delta_offset;

export import pragma.math;

export namespace pragma::math {
	struct DLLNETWORK DeltaOffset {
		DeltaOffset(Vector3 poffset, double ptime, double pdelta = 0.f);
		double time = 0.0;
		double delta = 0.0;
		Vector3 offset = {};
	};
}
