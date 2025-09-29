// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"
#include "mathutil/uvec.h"
#include "mathutil/uquat.h"


export module pragma.shared:scripting.lua.libraries.bounding_volume;

export namespace Lua::boundingvolume {
	DLLNETWORK luabind::mult<Vector3, Vector3> GetRotatedAABB(lua_State *l, const Vector3 &min, const Vector3 &max, const Mat3 &rot);
	DLLNETWORK luabind::mult<Vector3, Vector3> GetRotatedAABB(lua_State *l, const Vector3 &min, const Vector3 &max, const Quat &rot);
};
