/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lboundingvolume.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_mat4.h"
#include <pragma/math/intersection.h>
#include <mathutil/boundingvolume.h>

luabind::mult<Vector3,Vector3> Lua::boundingvolume::GetRotatedAABB(lua_State *l,const Vector3 &min,const Vector3 &max,const Mat4 &rot)
{
	Vector3 rmin;
	Vector3 rmax;
	bounding_volume::AABB::GetRotatedBounds(min,max,rot,&rmin,&rmax);
	return {l,rmin,rmax};
}
