// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "mathutil/umath.h"

#include "mathutil/uvec.h"

#include <mathutil/umat.h>
#include "luasystem.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

module pragma.shared;

import :scripting.lua.classes.quaternion;

Quat Lua::Quaternion::Copy(lua_State *l, const Quat &rot) { return rot; }

EulerAngles Lua::Quaternion::ToEulerAngles(lua_State *l, Quat &q, uint32_t rotationOrder)
{
	//EulerAngles ang(*q);
	//luabind::object(l,ang).push(l);
	EulerAngles ang {};
	auto m = glm::mat4_cast(q);
	switch(static_cast<pragma::RotationOrder>(rotationOrder)) {
	case pragma::RotationOrder::XYZ:
		glm::extractEulerAngleXYZ(m, ang.p, ang.y, ang.r);
		break;
	case pragma::RotationOrder::YXZ:
		glm::extractEulerAngleYXZ(m, ang.y, ang.p, ang.r);
		break;
	case pragma::RotationOrder::XZX:
		glm::extractEulerAngleXZX(m, ang.p, ang.r, ang.p);
		break;
	case pragma::RotationOrder::XYX:
		glm::extractEulerAngleXYX(m, ang.p, ang.y, ang.p);
		break;
	case pragma::RotationOrder::YXY:
		glm::extractEulerAngleYXY(m, ang.y, ang.p, ang.y);
		break;
	case pragma::RotationOrder::YZY:
		glm::extractEulerAngleYZY(m, ang.y, ang.r, ang.y);
		break;
	case pragma::RotationOrder::ZYZ:
		glm::extractEulerAngleZYZ(m, ang.r, ang.y, ang.r);
		break;
	case pragma::RotationOrder::ZXZ:
		glm::extractEulerAngleZXZ(m, ang.r, ang.p, ang.r);
		break;
	case pragma::RotationOrder::XZY:
		glm::extractEulerAngleXZY(m, ang.p, ang.r, ang.y);
		break;
	case pragma::RotationOrder::YZX:
		glm::extractEulerAngleYZX(m, ang.y, ang.r, ang.p);
		break;
	case pragma::RotationOrder::ZYX:
		glm::extractEulerAngleZYX(m, ang.r, ang.y, ang.p);
		break;
	case pragma::RotationOrder::ZXY:
		glm::extractEulerAngleZXY(m, ang.r, ang.p, ang.y);
		break;
	}
	ang.p = umath::rad_to_deg(ang.p);
	ang.y = umath::rad_to_deg(ang.y);
	ang.r = umath::rad_to_deg(ang.r);
	return ang;
}

luabind::mult<Vector3, float> Lua::Quaternion::ToAxisAngle(lua_State *l, const Quat &rot)
{
	Vector3 axis {};
	float angle = 0.f;
	uquat::to_axis_angle(rot, axis, angle);
	return {l, axis, angle};
}

luabind::mult<Vector3, Vector3, Vector3> Lua::Quaternion::GetOrientation(lua_State *l, const Quat &rot)
{
	Vector3 forward, right, up;
	uquat::get_orientation(rot, &forward, &right, &up);
	return {l, forward, right, up};
}

void Lua::Quaternion::Set(lua_State *, Quat &q, float w, float x, float y, float z)
{
	q.w = w;
	q.x = x;
	q.y = y;
	q.z = z;
}
static luabind::mult<Quat, Vector2> approach_direction(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit = nullptr, const ::Vector2 *yawLimit = nullptr, const Quat *rotPivot = nullptr,
  const EulerAngles *angOffset = nullptr)
{
	Vector2 resAm;
	auto r = uquat::approach_direction(rot, up, targetDir, rotAmount, &resAm, pitchLimit, yawLimit, rotPivot, angOffset);
	return {l, r, resAm};
}
luabind::mult<Quat, Vector2> Lua::Quaternion::ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit, const ::Vector2 *yawLimit, const Quat *rotPivot, const EulerAngles *angOffset)
{
	return approach_direction(l, rot, up, targetDir, rotAmount, pitchLimit, yawLimit, rotPivot, angOffset);
}
luabind::mult<Quat, Vector2> Lua::Quaternion::ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit, const ::Vector2 *yawLimit, const Quat *rotPivot)
{
	return approach_direction(l, rot, up, targetDir, rotAmount, pitchLimit, yawLimit, rotPivot);
}
luabind::mult<Quat, Vector2> Lua::Quaternion::ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit, const ::Vector2 *yawLimit)
{
	return approach_direction(l, rot, up, targetDir, rotAmount, pitchLimit, yawLimit);
}
luabind::mult<Quat, Vector2> Lua::Quaternion::ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount, const ::Vector2 *pitchLimit) { return approach_direction(l, rot, up, targetDir, rotAmount, pitchLimit); }
luabind::mult<Quat, Vector2> Lua::Quaternion::ApproachDirection(lua_State *l, const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const ::Vector2 &rotAmount) { return approach_direction(l, rot, up, targetDir, rotAmount); }
