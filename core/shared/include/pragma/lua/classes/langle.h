// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LANGLE_H__
#define __LANGLE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/math/angle/wvangle.h>
#include <pragma/math/angle/wvquaternion.h>
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>

namespace Lua {
	namespace Angle {
		DLLNETWORK luabind::mult<Vector3, Vector3, Vector3> Orientation(lua_State *l, const EulerAngles &ang);
		DLLNETWORK EulerAngles Copy(lua_State *l, const EulerAngles &ang);
		DLLNETWORK void ToQuaternion(lua_State *l, const EulerAngles &ang, uint32_t rotationOrder);
		DLLNETWORK void Set(lua_State *l, EulerAngles &ang, float p, float y, float r);
	};
};

#endif
