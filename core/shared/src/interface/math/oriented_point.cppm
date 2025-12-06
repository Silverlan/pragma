// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.oriented_point;

export import pragma.math;

export namespace pragma::math {
	class DLLNETWORK OrientedPoint {
	public:
		OrientedPoint();
		OrientedPoint(Vector3 &position);
		OrientedPoint(Quat &orientation);
		OrientedPoint(Vector3 &position, Quat &orientation);
		Vector3 pos = {};
		Quat rot = uquat::identity();
	};
}
