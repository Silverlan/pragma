/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lboundingvolume.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_mat4.h"
#include <pragma/math/intersection.h>
DLLNETWORK int Lua_boundingvolume_GetRotatedAABB(lua_State *l)
{
	Vector3 *min = _lua_Vector_check(l,1);
	Vector3 *max = _lua_Vector_check(l,2);
	Mat4 *rot = _lua_Mat4_check(l,3);
	Vector3 rmin,rmax;
	AABB::GetRotatedBounds(*min,*max,*rot,&rmin,&rmax);
	luabind::object(l,rmin).push(l);
	luabind::object(l,rmax).push(l);
	return 2;
}