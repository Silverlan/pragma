#include "stdafx_shared.h"
#include "pragma/lua/classes/lquaternion.h"
#include "pragma/lua/classes/langle.h"
#include <pragma/math/angle/wvangle.h>
#include <mathutil/umat.h>
#include "luasystem.h"
#include "pragma/util/util_approach_rotation.hpp"
#include "pragma/game/game_coordinate_system.hpp"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

void Lua::Quaternion::Copy(lua_State *l,Quat *q) {luabind::object(l,*q).push(l);}

void Lua::Quaternion::ToMatrix(lua_State *l,Quat *q)
{
	Lua::Push<Mat4>(l,glm::toMat4(*q));
	//luabind::object(l,umat::create(*q)).push(l);
}

void Lua::Quaternion::ToEulerAngles(lua_State *l,Quat &q,uint32_t rotationOrder)
{
	//EulerAngles ang(*q);
	//luabind::object(l,ang).push(l);
	EulerAngles ang {};
	auto m = glm::mat4_cast(q);
	switch(static_cast<pragma::RotationOrder>(rotationOrder))
	{
		case pragma::RotationOrder::XYZ:
			glm::extractEulerAngleXYZ(m,ang.p,ang.y,ang.r);
			break;
		case pragma::RotationOrder::YXZ:
			glm::extractEulerAngleYXZ(m,ang.y,ang.p,ang.r);
			break;
		case pragma::RotationOrder::XZX:
			glm::extractEulerAngleXZX(m,ang.p,ang.r,ang.p);
			break;
		case pragma::RotationOrder::XYX:
			glm::extractEulerAngleXYX(m,ang.p,ang.y,ang.p);
			break;
		case pragma::RotationOrder::YXY:
			glm::extractEulerAngleYXY(m,ang.y,ang.p,ang.y);
			break;
		case pragma::RotationOrder::YZY:
			glm::extractEulerAngleYZY(m,ang.y,ang.r,ang.y);
			break;
		case pragma::RotationOrder::ZYZ:
			glm::extractEulerAngleZYZ(m,ang.r,ang.y,ang.r);
			break;
		case pragma::RotationOrder::ZXZ:
			glm::extractEulerAngleZXZ(m,ang.r,ang.p,ang.r);
			break;
		case pragma::RotationOrder::XZY:
			glm::extractEulerAngleXZY(m,ang.p,ang.r,ang.y);
			break;
		case pragma::RotationOrder::YZX:
			glm::extractEulerAngleYZX(m,ang.y,ang.r,ang.p);
			break;
		case pragma::RotationOrder::ZYX:
			glm::extractEulerAngleZYX(m,ang.r,ang.y,ang.p);
			break;
		case pragma::RotationOrder::ZXY:
			glm::extractEulerAngleZXY(m,ang.r,ang.p,ang.y);
			break;
	}
	ang.p = umath::rad_to_deg(ang.p);
	ang.y = umath::rad_to_deg(ang.y);
	ang.r = umath::rad_to_deg(ang.r);
	Lua::Push<EulerAngles>(l,ang);
}

void Lua::Quaternion::ToAxisAngle(lua_State *l,Quat *q)
{
	Vector3 axis {};
	float angle = 0.f;
	uquat::to_axis_angle(*q,axis,angle);
	Lua::Push<Vector3>(l,axis);
	Lua::PushNumber(l,angle);
}

void Lua::Quaternion::Inverse(lua_State*,Quat *q) {uquat::inverse(*q);}

void Lua::Quaternion::GetInverse(lua_State *l,Quat *q)
{
	Quat inv = uquat::get_inverse(*q);
	luabind::object(l,inv).push(l);
}

void Lua::Quaternion::GetForward(lua_State *l,Quat *q)
{
	Vector3 forward = uquat::forward(*q);
	luabind::object(l,forward).push(l);
}

void Lua::Quaternion::GetRight(lua_State *l,Quat *q)
{
	Vector3 right = uquat::right(*q);
	luabind::object(l,right).push(l);
}

void Lua::Quaternion::GetUp(lua_State *l,Quat *q)
{
	Vector3 up = uquat::up(*q);
	luabind::object(l,up).push(l);
}

void Lua::Quaternion::GetOrientation(lua_State *l,Quat *q)
{
	Vector3 forward,right,up;
	uquat::get_orientation(*q,&forward,&right,&up);
	luabind::object(l,forward).push(l);
	luabind::object(l,right).push(l);
	luabind::object(l,up).push(l);
}

void Lua::Quaternion::Lerp(lua_State *l,Quat &qa,Quat &qb,float factor)
{
	luabind::object(l,uquat::lerp(qa,qb,factor)).push(l);
}

void Lua::Quaternion::Slerp(lua_State *l,Quat &qa,Quat &qb,float factor)
{
	luabind::object(l,uquat::slerp(qa,qb,factor)).push(l);
}

void Lua::Quaternion::DotProduct(lua_State *l,Quat &q,Quat &qb)
{
	Lua::PushNumber(l,uquat::dot_product(q,qb));
}

void Lua::Quaternion::Length(lua_State *l,Quat &q)
{
	Lua::PushNumber(l,uquat::length(q));
}

void Lua::Quaternion::Normalize(lua_State*,Quat &q)
{
	uquat::normalize(q);
}

void Lua::Quaternion::GetNormal(lua_State *l,Quat &q)
{
	luabind::object(l,uquat::get_normal(q)).push(l);
}

void Lua::Quaternion::Set(lua_State*,Quat &q,float w,float x,float y,float z)
{
	q.w = w;
	q.x = x;
	q.y = y;
	q.z = z;
}
void Lua::Quaternion::GetConjugate(lua_State *l,Quat &q)
{
	Lua::Push<Quat>(l,glm::conjugate(q));
}
static void approach_direction(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit=nullptr,const ::Vector2 *yawLimit=nullptr,const Quat *rotPivot=nullptr,const EulerAngles *angOffset=nullptr)
{
	Vector2 resAm;
	auto r = uquat::approach_direction(rot,up,targetDir,rotAmount,&resAm,pitchLimit,yawLimit,rotPivot,angOffset);
	Lua::Push<Quat>(l,r);
	Lua::Push<Vector2>(l,resAm);
}
void Lua::Quaternion::ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit,const ::Vector2 *yawLimit,const Quat *rotPivot,const EulerAngles *angOffset) {approach_direction(l,rot,up,targetDir,rotAmount,pitchLimit,yawLimit,rotPivot,angOffset);}
void Lua::Quaternion::ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit,const ::Vector2 *yawLimit,const Quat *rotPivot) {approach_direction(l,rot,up,targetDir,rotAmount,pitchLimit,yawLimit,rotPivot);}
void Lua::Quaternion::ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit,const ::Vector2 *yawLimit) {approach_direction(l,rot,up,targetDir,rotAmount,pitchLimit,yawLimit);}
void Lua::Quaternion::ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit) {approach_direction(l,rot,up,targetDir,rotAmount,pitchLimit);}
void Lua::Quaternion::ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount) {approach_direction(l,rot,up,targetDir,rotAmount);}
