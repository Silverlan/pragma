// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.sphere;

export import pragma.math;

export namespace pragma::math {
	struct DLLNETWORK Sphere {
		Sphere() : pos(0, 0, 0), radius(0.f) {}
		Sphere(Vector3 &position, float r) : pos(position), radius(r) {}
		Vector3 pos = {};
		float radius = 0.f;
	};
}
