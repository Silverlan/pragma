/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LBOUNDINGVOLUME_H__
#define __LBOUNDINGVOLUME_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua::boundingvolume
{
	DLLNETWORK luabind::mult<Vector3,Vector3> GetRotatedAABB(lua_State *l,const Vector3 &min,const Vector3 &max,const Mat3 &rot);
	DLLNETWORK luabind::mult<Vector3,Vector3> GetRotatedAABB(lua_State *l,const Vector3 &min,const Vector3 &max,const Quat &rot);
};

#endif
