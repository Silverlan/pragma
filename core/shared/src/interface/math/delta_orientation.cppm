// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.delta_orientation;

export import pragma.math;

export namespace pragma::math {
	struct DLLNETWORK DeltaOrientation {
		DeltaOrientation(Quat porientation, double ptime, double pdelta = 0.f);
		double time = 0.0;
		double delta = 0.0;
		Quat orientation = uquat::identity();
	};
}
