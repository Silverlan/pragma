// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.angle;

luabind::mult<Vector3, Vector3, Vector3> Lua::Angle::Orientation(lua::State *l, const EulerAngles &ang)
{
	Vector3 forward, right, up;
	ang.GetOrientation(&forward, &right, &up);
	return {l, forward, right, up};
}

EulerAngles Lua::Angle::Copy(lua::State *l, const EulerAngles &ang) { return ang; }

void Lua::Angle::ToQuaternion(lua::State *l, const EulerAngles &ang, uint32_t rotationOrder)
{
	auto m = umat::identity();
	switch(static_cast<pragma::RotationOrder>(rotationOrder)) {
	case pragma::RotationOrder::XYZ:
		m = glm::gtx::eulerAngleXYZ(pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.r));
		break;
	case pragma::RotationOrder::YXZ:
		m = glm::gtx::eulerAngleYXZ(pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.r));
		break;
	case pragma::RotationOrder::XZX:
		m = glm::gtx::eulerAngleXZX(pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.p));
		break;
	case pragma::RotationOrder::XYX:
		m = glm::gtx::eulerAngleXYX(pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.p));
		break;
	case pragma::RotationOrder::YXY:
		m = glm::gtx::eulerAngleYXY(pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.y));
		break;
	case pragma::RotationOrder::YZY:
		m = glm::gtx::eulerAngleYZY(pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.y));
		break;
	case pragma::RotationOrder::ZYZ:
		m = glm::gtx::eulerAngleZYZ(pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.r));
		break;
	case pragma::RotationOrder::ZXZ:
		m = glm::gtx::eulerAngleZXZ(pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.r));
		break;
	case pragma::RotationOrder::XZY:
		m = glm::gtx::eulerAngleXZY(pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.y));
		break;
	case pragma::RotationOrder::YZX:
		m = glm::gtx::eulerAngleYZX(pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.p));
		break;
	case pragma::RotationOrder::ZYX:
		m = glm::gtx::eulerAngleZYX(pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.p));
		break;
	case pragma::RotationOrder::ZXY:
		m = glm::gtx::eulerAngleZXY(pragma::math::deg_to_rad(ang.r), pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.y));
		break;
	}
	auto q = glm::quat_cast(m);
	luabind::object(l, q).push(l);
}

void Lua::Angle::Set(lua::State *, EulerAngles &ang, float p, float y, float r)
{
	ang.p = p;
	ang.y = y;
	ang.r = r;
}
