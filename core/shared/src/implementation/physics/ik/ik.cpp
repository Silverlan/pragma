// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <mathutil/transform.hpp>

module pragma.shared;

import :physics.ik;

void util::ik::get_local_transform(const Node &node, umath::Transform &t)
{
	auto rot = get_rotation(node);
	t.SetRotation(rot);
	t.SetOrigin(Vector3 {node.r.x, node.r.y, node.r.z});
}

Quat util::ik::get_rotation(const Node &node)
{
	auto axis = Vector3(node.v.x, node.v.y, node.v.z);
	auto rot = uquat::identity();
	if(uvec::length_sqr(axis) > 0.f)
		rot = uquat::create(axis, node.GetTheta());
	return rot;
}
