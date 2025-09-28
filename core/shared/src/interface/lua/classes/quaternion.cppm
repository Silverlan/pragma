// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"
#include "mathutil/uquat.h"

export module pragma.shared:scripting.lua.classes.quaternion;

export namespace Lua {
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
