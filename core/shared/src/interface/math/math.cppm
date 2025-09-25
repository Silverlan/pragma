// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.shared:math;

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
export import :math.pid_controller;
export import :math.random;
export import :math.seb;
export import :math.sphere;
export import :math.surface_material;
export import :math.triangulate;

export namespace util {
	DLLNETWORK Vector3 angular_velocity_to_linear(const Vector3 &refPos, const Vector3 &angVel, const Vector3 &tgtPos);
};
