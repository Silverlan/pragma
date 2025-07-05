// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LBOUNDINGVOLUME_H__
#define __LBOUNDINGVOLUME_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua::boundingvolume {
	DLLNETWORK luabind::mult<Vector3, Vector3> GetRotatedAABB(lua_State *l, const Vector3 &min, const Vector3 &max, const Mat3 &rot);
	DLLNETWORK luabind::mult<Vector3, Vector3> GetRotatedAABB(lua_State *l, const Vector3 &min, const Vector3 &max, const Quat &rot);
};

#endif
