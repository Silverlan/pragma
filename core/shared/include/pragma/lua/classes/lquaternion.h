/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LQUATERNION_H__
#define __LQUATERNION_H__

#include "pragma/networkdefinitions.h"
#include <pragma/math/angle/wvquaternion.h>
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace Quaternion
	{
		DLLNETWORK void Copy(lua_State *l,Quat *q);
		DLLNETWORK void ToMatrix(lua_State *l,Quat *q);
		DLLNETWORK void ToEulerAngles(lua_State *l,Quat &q,uint32_t rotationOrder);
		DLLNETWORK void ToAxisAngle(lua_State *l,Quat *q);
		DLLNETWORK void GetForward(lua_State *l,Quat *q);
		DLLNETWORK void GetRight(lua_State *l,Quat *q);
		DLLNETWORK void GetUp(lua_State *l,Quat *q);
		DLLNETWORK void GetOrientation(lua_State *l,Quat *q);
		DLLNETWORK void Inverse(lua_State *l,Quat *q);
		DLLNETWORK void GetInverse(lua_State *l,Quat *q);
		DLLNETWORK void Lerp(lua_State *l,Quat &qa,Quat &qb,float factor);
		DLLNETWORK void Slerp(lua_State *l,Quat &qa,Quat &qb,float factor);
		DLLNETWORK void DotProduct(lua_State *l,Quat &q,Quat &qb);
		DLLNETWORK void Length(lua_State *l,Quat &q);
		DLLNETWORK void Normalize(lua_State *l,Quat &q);
		DLLNETWORK void GetNormal(lua_State *l,Quat &q);
		DLLNETWORK void Set(lua_State *l,Quat &q,float w,float x,float y,float z);
		DLLNETWORK void GetConjugate(lua_State *l,Quat &q);
		DLLNETWORK void ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit,const ::Vector2 *yawLimit,const Quat *rotPivot,const EulerAngles *angOffset);
		DLLNETWORK void ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit,const ::Vector2 *yawLimit,const Quat *rotPivot);
		DLLNETWORK void ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit,const ::Vector2 *yawLimit);
		DLLNETWORK void ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount,const ::Vector2 *pitchLimit);
		DLLNETWORK void ApproachDirection(lua_State *l,const Quat &rot,const Vector3 &up,const Vector3 &targetDir,const ::Vector2 &rotAmount);
	};
};

#endif
