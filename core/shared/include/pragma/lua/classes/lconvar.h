/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LCONVAR_H__
#define __LCONVAR_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <pragma/console/convars.h>
DLLNETWORK int Lua_cvar_CreateConVar(lua_State *l);
DLLNETWORK int Lua_cvar_CreateConCommand(lua_State *l);
DLLNETWORK int Lua_cvar_GetConVar(lua_State *l);
DLLNETWORK int Lua_cvar_GetConVarInt(lua_State *l);
DLLNETWORK int Lua_cvar_GetConVarFloat(lua_State *l);
DLLNETWORK int Lua_cvar_GetConVarString(lua_State *l);
DLLNETWORK int Lua_cvar_GetConVarBool(lua_State *l);
DLLNETWORK int Lua_cvar_GetConVarFlags(lua_State *l);
DLLNETWORK int Lua_cvar_Run(lua_State *l);
DLLNETWORK int Lua_cvar_AddChangeCallback(lua_State *l);

////////////////////////////////////

DLLNETWORK void Lua_ConVar_GetString(lua_State *l,ConVar *cvar);
DLLNETWORK void Lua_ConVar_GetInt(lua_State *l,ConVar *cvar);
DLLNETWORK void Lua_ConVar_GetFloat(lua_State *l,ConVar *cvar);
DLLNETWORK void Lua_ConVar_GetBool(lua_State *l,ConVar *cvar);
//DLLNETWORK void Lua_ConVar_SetValue(lua_State *l,ConVar *cvar,std::string value);
DLLNETWORK void Lua_ConVar_GetFlags(lua_State *l,ConVar *cvar);

namespace Lua
{
	namespace console
	{
		DLLNETWORK int register_override(lua_State *l);
		DLLNETWORK int clear_override(lua_State *l);
		DLLNETWORK int parse_command_arguments(lua_State *l);
	};
};

#endif
