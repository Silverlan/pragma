// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.quaternion;

export import :scripting.lua.api;
export import pragma.math;

export namespace Lua {
	namespace Quaternion {
		DLLNETWORK Quat Copy(lua::State *l, const Quat &rot);
		DLLNETWORK EulerAngles ToEulerAngles(lua::State *l, Quat &q, uint32_t rotationOrder);
		DLLNETWORK luabind::mult<Vector3, float> ToAxisAngle(lua::State *l, const Quat &rot);
		DLLNETWORK luabind::mult<Vector3, Vector3, Vector3> GetOrientation(lua::State *l, const Quat &rot);
		DLLNETWORK void Set(lua::State *l, Quat &q, float w, float x, float y, float z);
		DLLNETWORK luabind::mult<Quat, Vector2> ApproachDirection(lua::State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount, const Vector2 *pitchLimit, const Vector2 *yawLimit, const Quat *rotPivot, const EulerAngles *angOffset);
		DLLNETWORK luabind::mult<Quat, Vector2> ApproachDirection(lua::State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount, const Vector2 *pitchLimit, const Vector2 *yawLimit, const Quat *rotPivot);
		DLLNETWORK luabind::mult<Quat, Vector2> ApproachDirection(lua::State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount, const Vector2 *pitchLimit, const Vector2 *yawLimit);
		DLLNETWORK luabind::mult<Quat, Vector2> ApproachDirection(lua::State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount, const Vector2 *pitchLimit);
		DLLNETWORK luabind::mult<Quat, Vector2> ApproachDirection(lua::State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount);
	};
};
