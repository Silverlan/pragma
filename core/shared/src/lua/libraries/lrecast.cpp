/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/entities/baseworld.h"
#include "pragma/lua/libraries/lrecast.h"
#include "luasystem.h"
#include "pragma/ai/navsystem.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/classes/ldef_vector.h"

extern DLLENGINE Engine *engine;

DLLNETWORK int Lua_recast_Test(lua_State *l)
{
	// TODO: Remove this
	/*Vector3 start(-755.f,-135.f,538.f);
	Vector3 end(175.f,-135.f,-269.f);
	RcPathResult *res;
	if(!NavSystem::FindPath(start,end,&res))
		return 0;
	lua_newtable(l);
	int top = lua_gettop(l);
	int n = 1;
	Vector3 nextPoint = res->start;
	for(auto i=decltype(res->pathCount){0};i<res->pathCount;++i)
	{
		Vector3 pathPoint = nextPoint;
		//res->query->closestPointOnPoly(
		//	res->path[i],&pathPoint[0],&nextPoint[0]
		//);
		Lua::Push<Vector3>(l,pathPoint);
		lua_rawseti(l,top,n);
		n++;
	}
	Lua::Push<Vector3>(l,res->end);
	lua_rawseti(l,top,n);
	return 1;*/
	return 0;
}
