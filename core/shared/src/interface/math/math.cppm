// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math;

export import :math.approach_rotation;
export import :math.ballistic;
export import :math.best_fitting_plane;
export import :math.bounding_node;
export import :math.delta_offset;
export import :math.delta_orientation;
export import :math.delta_transform;
export import :math.easing;
export import :math.frustum;
export import :math.hermite;
export import :math.ico_sphere;
export import :math.intersection;
export import :math.noise;
export import :math.octtree_node;
export import :math.orientation;
export import :math.oriented_point;
export import :math.pid_controller;
export import :math.print;
export import :math.random;
export import :math.seb;
export import :math.sphere;
export import :math.triangulate;

export {
	namespace pragma::math {
		DLLNETWORK Vector3 angular_velocity_to_linear(const Vector3 &refPos, const Vector3 &angVel, const Vector3 &tgtPos);
		DLLNETWORK uint32_t find_bezier_roots(float x, float v0, float v1, float v2, float v3, std::array<float, 3> &roots);
		DLLNETWORK float calc_bezier_point(float f1, float f2, float f3, float f4, float t);
	};
}
