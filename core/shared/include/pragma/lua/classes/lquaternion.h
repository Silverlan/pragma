/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LQUATERNION_H__
#define __LQUATERNION_H__

#include "pragma/networkdefinitions.h"
#include <pragma/math/angle/wvquaternion.h>
#include <pragma/lua/luaapi.h>

namespace Lua {
	namespace Quaternion {
		DLLNETWORK Quat Copy(lua_State *l, const Quat &rot);
		DLLNETWORK EulerAngles ToEulerAngles(lua_State *l, Quat &q, uint32_t rotationOrder);
		DLLNETWORK luabind::mult<Vector3, float> ToAxisAngle(lua_State *l, const Quat &rot);
		DLLNETWORK luabind::mult<Vector3, Vector3, Vector3> GetOrientation(lua_State *l, const Quat &rot);
		DLLNETWORK void Set(lua_State *l, Quat &q, float w, float x, float y, float z);
		DLLNETWORK luabind::mult<Quat, ::Vector2> ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit, const ::Vector2 *yawLimit, const Quat *rotPivot, const EulerAngles *angOffset);
		DLLNETWORK luabind::mult<Quat, ::Vector2> ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit, const ::Vector2 *yawLimit, const Quat *rotPivot);
		DLLNETWORK luabind::mult<Quat, ::Vector2> ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit, const ::Vector2 *yawLimit);
		DLLNETWORK luabind::mult<Quat, ::Vector2> ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit);
		DLLNETWORK luabind::mult<Quat, ::Vector2> ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount);
	};
};

#endif
