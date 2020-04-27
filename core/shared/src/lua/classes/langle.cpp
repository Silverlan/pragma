/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/langle.h"
#include "pragma/game/game_coordinate_system.hpp"
#include <glm/gtx/euler_angles.hpp>

Vector3 Lua::Angle::Up(EulerAngles *ang) {return ang->Up();}

void Lua::Angle::Orientation(lua_State *l,EulerAngles *ang)
{
	Vector3 forward,right,up;
	ang->GetOrientation(&forward,&right,&up);
	luabind::object(l,forward).push(l);
	luabind::object(l,right).push(l);
	luabind::object(l,up).push(l);
}

void Lua::Angle::Copy(lua_State *l,EulerAngles *ang) {luabind::object(l,*ang).push(l);}

void Lua::Angle::ToMatrix(lua_State *l,EulerAngles *ang)
{
	Mat4 mat = ang->ToMatrix();
	luabind::object(l,mat).push(l);
}

void Lua::Angle::ToQuaternion(lua_State *l,EulerAngles *ang,uint32_t rotationOrder)
{
	auto m = umat::identity();
	switch(static_cast<pragma::RotationOrder>(rotationOrder))
	{
		case pragma::RotationOrder::XYZ:
			m = glm::eulerAngleXYZ(umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->r));
			break;
		case pragma::RotationOrder::YXZ:
			m = glm::eulerAngleYXZ(umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->r));
			break;
		case pragma::RotationOrder::XZX:
			m = glm::eulerAngleXZX(umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->p));
			break;
		case pragma::RotationOrder::XYX:
			m = glm::eulerAngleXYX(umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->p));
			break;
		case pragma::RotationOrder::YXY:
			m = glm::eulerAngleYXY(umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->y));
			break;
		case pragma::RotationOrder::YZY:
			m = glm::eulerAngleYZY(umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->y));
			break;
		case pragma::RotationOrder::ZYZ:
			m = glm::eulerAngleZYZ(umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->r));
			break;
		case pragma::RotationOrder::ZXZ:
			m = glm::eulerAngleZXZ(umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->r));
			break;
		case pragma::RotationOrder::XZY:
			m = glm::eulerAngleXZY(umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->y));
			break;
		case pragma::RotationOrder::YZX:
			m = glm::eulerAngleYZX(umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->p));
			break;
		case pragma::RotationOrder::ZYX:
			m = glm::eulerAngleZYX(umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->y),umath::deg_to_rad(ang->p));
			break;
		case pragma::RotationOrder::ZXY:
			m = glm::eulerAngleZXY(umath::deg_to_rad(ang->r),umath::deg_to_rad(ang->p),umath::deg_to_rad(ang->y));
			break;
	}
	auto q = glm::quat_cast(m);
	luabind::object(l,q).push(l);
}

void Lua::Angle::Set(lua_State*,EulerAngles *ang,float p,float y,float r)
{
	ang->p = p;
	ang->y = y;
	ang->r = r;
}