/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LPRINT_H__
#define __LPRINT_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua::console
{
	DLLNETWORK int print(lua_State *l);
	DLLNETWORK int print_table(lua_State *l,std::string tab,int idx=1);
	DLLNETWORK int print_table(lua_State *l);
	DLLNETWORK int msg(lua_State *l,int st);
	DLLNETWORK int msg(lua_State *l);
	DLLNETWORK int msgn(lua_State *l);
	DLLNETWORK int msgc(lua_State *l);
	DLLNETWORK int msgw(lua_State *l);
	DLLNETWORK int msge(lua_State *l);
};

namespace Lua::log
{
	DLLNETWORK int info(lua_State *l);
	DLLNETWORK int warn(lua_State *l);
	DLLNETWORK int error(lua_State *l);
	DLLNETWORK int critical(lua_State *l);
	DLLNETWORK int debug(lua_State *l);
	DLLNETWORK int color(lua_State *l);
};

namespace Lua::debug
{
	DLLNETWORK int print(lua_State *l);
};

#endif
