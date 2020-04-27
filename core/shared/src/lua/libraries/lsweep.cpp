/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lsweep.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <pragma/math/intersection.h>
#include "luasystem.h"
DLLNETWORK int Lua_sweep_AABBWithAABB(lua_State *l)
{
	Vector3 originA = *_lua_Vector_check(l,1);
	Vector3 endA = *_lua_Vector_check(l,2);
	Vector3 extentsA = *_lua_Vector_check(l,3);
	Vector3 originB = *_lua_Vector_check(l,4);
	Vector3 endB = *_lua_Vector_check(l,5);
	Vector3 extentsB = *_lua_Vector_check(l,6);
	float tEntry,tExit;
	Vector3 n;
	bool b = Sweep::AABBWithAABB(originA,endA,extentsA,originB,endB,extentsB,&tEntry,&tExit,&n);
	lua_pushboolean(l,b);
	if(b)
	{
		Lua::PushNumber(l,tEntry);
		Lua::PushNumber(l,tExit);
		luabind::object(l,n).push(l);
		return 4;
	}
	return 1;
}

DLLNETWORK int Lua_sweep_AABBWithPlane(lua_State *l)
{
	Vector3 ext = *_lua_Vector_check(l,1);
	Vector3 origin = *_lua_Vector_check(l,2);
	Vector3 dir = *_lua_Vector_check(l,3);
	Vector3 n = *_lua_Vector_check(l,4);
	float d = static_cast<float>(Lua::CheckNumber(l,5));
	float t;
	bool b = Sweep::AABBWithPlane(origin,dir,ext,n,d,&t);
	lua_pushboolean(l,b);
	if(b)
	{
		Lua::PushNumber(l,t);
		return 2;
	}
	return 1;
}