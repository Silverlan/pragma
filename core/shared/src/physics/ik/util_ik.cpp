#include "stdafx_shared.h"
#include "pragma/physics/ik/util_ik.hpp"
#include "pragma/physics/phystransform.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/buss_ik/Tree.h"

void util::ik::get_local_transform(const Node &node,PhysTransform &t)
{
	auto rot = get_rotation(node);
	t.SetRotation(rot);
	t.SetOrigin(Vector3(node.r.x,node.r.y,node.r.z) /static_cast<float>(PhysEnv::WORLD_SCALE));
}

Quat util::ik::get_rotation(const Node &node)
{
	auto axis = Vector3(node.v.x,node.v.y,node.v.z);
	auto rot = uquat::identity();
	if(uvec::length_sqr(axis) > 0.f)
		rot = uquat::create(axis,node.GetTheta());
	return rot;
}
