/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LSWEEP_H__
#define __LSWEEP_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
DLLNETWORK int Lua_sweep_AABBWithAABB(lua_State *l);
DLLNETWORK int Lua_sweep_AABBWithPlane(lua_State *l);
#endif