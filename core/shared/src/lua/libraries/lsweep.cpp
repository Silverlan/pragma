// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lsweep.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <pragma/math/intersection.h>
#include "luasystem.h"
void Lua::sweep::AABBWithAABB(const Vector3 &originA, const Vector3 &endA, const Vector3 &extentsA, const Vector3 &originB, const Vector3 &endB, const Vector3 &extentsB, bool &outResult, float &outTEntry, float &outTExit, Vector3 &outNormal)
{
	outResult = umath::sweep::aabb_with_aabb(originA, endA, extentsA, originB, endB, extentsB, &outTEntry, &outTExit, &outNormal);
}

void Lua::sweep::AABBWithPlane(const Vector3 &ext, const Vector3 &origin, const Vector3 &dir, const Vector3 &n, float d, bool &outResult, float &outT) { outResult = umath::sweep::aabb_with_plane(origin, dir, ext, n, d, &outT); }
