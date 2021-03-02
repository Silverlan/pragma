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
DLLNETWORK int Lua_print(lua_State *l);
DLLNETWORK int Lua_PrintTable(lua_State *l,std::string tab,int idx=1);
DLLNETWORK int Lua_PrintTable(lua_State *l);
DLLNETWORK int Lua_Msg(lua_State *l,int st);
DLLNETWORK int Lua_Msg(lua_State *l);
DLLNETWORK int Lua_MsgN(lua_State *l);
DLLNETWORK int Lua_MsgC(lua_State *l);
DLLNETWORK int Lua_MsgW(lua_State *l);
DLLNETWORK int Lua_MsgE(lua_State *l);
#endif