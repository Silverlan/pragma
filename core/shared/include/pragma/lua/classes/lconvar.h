/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LCONVAR_H__
#define __LCONVAR_H__
#include "pragma/networkdefinitions.h"
#include "pragma/lua/types/udm.hpp"
#include <pragma/lua/luaapi.h>
#include <pragma/console/convars.h>
namespace Lua::console
{
	ConVar *CreateConVar(lua_State *l,const std::string &cmd,::udm::Type type,Lua::udm_type def,ConVarFlags flags=ConVarFlags::None,const std::string &help="");
	void CreateConCommand(lua_State *l,const std::string &name,const Lua::func<void,pragma::BasePlayerComponent,float,Lua::variadic<std::string>> &function,ConVarFlags flags,const std::string &help);
	void CreateConCommand(lua_State *l,const std::string &name,const Lua::func<void,pragma::BasePlayerComponent,float,Lua::variadic<std::string>> &function,ConVarFlags flags);
	void CreateConCommand(lua_State *l,const std::string &name,const Lua::func<void,pragma::BasePlayerComponent,float,Lua::variadic<std::string>> &function,const std::string &help);
	ConVar *GetConVar(lua_State *l,const std::string &name);
	int32_t GetConVarInt(lua_State *l,const std::string &conVar);
	float GetConVarFloat(lua_State *l,const std::string &conVar);
	std::string GetConVarString(lua_State *l,const std::string &conVar);
	bool GetConVarBool(lua_State *l,const std::string &conVar);
	ConVarFlags GetConVarFlags(lua_State *l,const std::string &conVar);
	DLLNETWORK int Run(lua_State *l);
	DLLNETWORK int AddChangeCallback(lua_State *l);
};

////////////////////////////////////

namespace Lua
{
	namespace console
	{
		DLLNETWORK void register_override(lua_State *l,const std::string &src,const std::string &dst);
		DLLNETWORK void clear_override(lua_State *l,const std::string &src);
		DLLNETWORK int parse_command_arguments(lua_State *l);
	};
};

#endif
