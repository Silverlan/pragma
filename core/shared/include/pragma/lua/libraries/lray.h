/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LRAY_H__
#define __LRAY_H__
#include "pragma/networkdefinitions.h"
#include "pragma/physics/shape.hpp"
#include "pragma/types.hpp"
#include <pragma/lua/luaapi.h>

class TraceData;
namespace pragma::physics {class IConvexShape;};
namespace Lua::TraceData
{
	DLLNETWORK void SetSource(lua_State *l,::TraceData &data,const pragma::physics::IConvexShape &shape);
	DLLNETWORK void SetFlags(lua_State *l,::TraceData &data,unsigned int flags);
	DLLNETWORK void SetFilter(lua_State *l,::TraceData &data,luabind::object);
	DLLNETWORK void SetCollisionFilterMask(lua_State*,::TraceData &data,unsigned int mask);
	DLLNETWORK void SetCollisionFilterGroup(lua_State*,::TraceData &data,unsigned int group);
	DLLNETWORK void GetSourceTransform(lua_State*,::TraceData &data);
	DLLNETWORK void GetTargetTransform(lua_State*,::TraceData &data);
	DLLNETWORK void GetSourceOrigin(lua_State*,::TraceData &data);
	DLLNETWORK void GetTargetOrigin(lua_State*,::TraceData &data);
	DLLNETWORK void GetSourceRotation(lua_State*,::TraceData &data);
	DLLNETWORK void GetTargetRotation(lua_State*,::TraceData &data);
	DLLNETWORK void GetDistance(lua_State*,::TraceData &data);
	DLLNETWORK void GetDirection(lua_State*,::TraceData &data);

	DLLNETWORK void FillTraceResultTable(lua_State *l,TraceResult &res);
};

#endif
