/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/classes/lconvar.h"
#include <pragma/engine.h>
#include "luasystem.h"
#include <pragma/game/game.h>
#include <mathutil/umath.h>
#include <pragma/console/command_options.hpp>

extern DLLNETWORK Engine *engine;
int Lua_cvar_CreateConVar(lua_State *l)
{
	auto cmd = Lua::CheckString(l,1);
	auto def = Lua::CheckString(l,2);
	auto flags = ConVarFlags::None;
	std::string help;
	if(Lua::IsSet(l,3))
		flags = Lua::CheckInt<ConVarFlags>(l,3);
	if(Lua::IsSet(l,4))
		help = Lua::CheckString(l,4);
	auto *state = engine->GetNetworkState(l);
	if(state == nullptr)
		return 0;
	auto *cvar = state->CreateConVar(cmd,def,flags,help);
	if(cvar == nullptr)
		return 0;
	Lua::Push<ConVar*>(l,cvar);
	return 1;
}

int Lua_cvar_CreateConCommand(lua_State *l)
{
	auto name = Lua::CheckString(l,1);
	Lua::CheckFunction(l,2);
	auto fc = luabind::object(luabind::from_stack(l,2));
	auto flags = ConVarFlags::None;
	std::string help;
	if(Lua::IsSet(l,3))
	{
		if(Lua::IsNumber(l,3) == false)
			help = Lua::CheckString(l,3);
		else
		{
			flags = static_cast<ConVarFlags>(Lua::CheckInt(l,3));
			if(Lua::IsSet(l,4) == true)
				help = Lua::CheckString(l,4);
		}
	}
	auto *state = engine->GetNetworkState(l);
	if(state == nullptr)
		return 0;
	state->CreateConCommand(name,fc,flags,help);
	return 0;
}

int Lua_cvar_GetConVar(lua_State *l)
{
	auto name = Lua::CheckString(l,1);
	auto *state = engine->GetNetworkState(l);
	if(state == nullptr)
		return 0;
	auto *cv = state->GetConVar(name);
	if(cv == nullptr)
		return 0;
	if(cv->GetType() != ConType::Var)
		return 0;
	auto *cvar = static_cast<ConVar*>(cv);
	Lua::Push<ConVar*>(l,cvar);
	return 1;
}

#define GetConVarType(lua_state,cmd,lua_push,valDef,dataType) \
	auto *state = engine->GetNetworkState(lua_state); \
	if(state == nullptr) \
		lua_push(lua_state,valDef); \
	else \
	{ \
		auto *cv = state->GetConVar(cmd); \
		if(cv == nullptr || cv->GetType() != ConType::Var) \
			lua_push(lua_state,valDef); \
		else \
		{ \
			auto *cvar = static_cast<ConVar*>(cv); \
			lua_push(lua_state,cvar->dataType); \
		} \
	} \
	return 1;


int Lua_cvar_GetConVarInt(lua_State *l)
{
	std::string cmd = luaL_checkstring(l,1);
	GetConVarType(l,cmd,Lua::PushInt,0,GetInt());
}

int Lua_cvar_GetConVarFloat(lua_State *l)
{
	std::string cmd = luaL_checkstring(l,1);
	GetConVarType(l,cmd,Lua::PushNumber,0,GetFloat());
}

int Lua_cvar_GetConVarString(lua_State *l)
{
	std::string cmd = luaL_checkstring(l,1);
	GetConVarType(l,cmd,lua_pushstring,"",GetString().c_str());
}

int Lua_cvar_GetConVarBool(lua_State *l)
{
	std::string cmd = luaL_checkstring(l,1);
	GetConVarType(l,cmd,lua_pushboolean,false,GetBool());
}

int Lua_cvar_GetConVarFlags(lua_State *l)
{
	std::string cmd = luaL_checkstring(l,1);
	auto name = Lua::CheckString(l,1);
	auto *state = engine->GetNetworkState(l);
	if(state == nullptr)
		return 0;
	auto *cv = state->GetConVar(name);
	if(cv == nullptr)
		return 0;
	Lua::PushInt(l,umath::to_integral(cv->GetFlags()));
	return 1;
}

int Lua_cvar_Run(lua_State *l)
{
	int argc = lua_gettop(l);
	std::string cmd = luaL_checkstring(l,1);
	int i = 2;
	while(argc >= i)
	{
		std::string arg = luaL_checkstring(l,i);
		cmd += " \"" +arg +"\"";
		i++;
	}
	//engine->ConsoleInput(cmd.c_str());
	engine->ProcessConsoleInput(cmd);
	return 0;
}

int Lua_cvar_AddChangeCallback(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	if(game == nullptr)
		return 0;
	auto cvar = Lua::CheckString(l,1);
	Lua::CheckFunction(l,2);
	auto fc = luabind::object(luabind::from_stack(l,2));
	game->AddConVarCallback(cvar,fc);
	return 0;
}

int Lua::console::register_override(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	std::string src = Lua::CheckString(l,1);
	std::string dst = Lua::CheckString(l,2);
	state->SetConsoleCommandOverride(src,dst);
	return 0;
}
int Lua::console::clear_override(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	std::string src = Lua::CheckString(l,1);
	state->ClearConsoleCommandOverride(src);
	return 0;
}
int Lua::console::parse_command_arguments(lua_State *l)
{
	int32_t tArgs = 1;
	Lua::CheckTable(l,tArgs);
	std::vector<std::string> args {};
	auto numArgs = Lua::GetObjectLength(l,tArgs);
	args.reserve(numArgs);
	for(auto i=decltype(numArgs){0u};i<numArgs;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */
		Lua::GetTableValue(l,tArgs); /* 1 */
		args.push_back(Lua::CheckString(l,-1));

		Lua::Pop(l,1); /* 0 */
	}

	std::unordered_map<std::string,pragma::console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(args,commandOptions);

	auto t = Lua::CreateTable(l);
	for(auto &pair : commandOptions)
	{
		Lua::PushString(l,pair.first); /* 1 */
		auto tCmd = Lua::CreateTable(l); /* 2 */
		auto &cmd = pair.second;
		int32_t argIdx = 1;
		for(auto &str : cmd.parameters)
		{
			Lua::PushInt(l,argIdx++); /* 3 */
			Lua::PushString(l,str); /* 4 */
			Lua::SetTableValue(l,tCmd); /* 2 */
		}
		Lua::SetTableValue(l,t); /* 0 */
	}
	return 1;
}

////////////////////////////////////

void Lua_ConVar_GetString(lua_State *l,ConVar *cvar) {lua_pushstring(l,cvar->GetString().c_str());}
void Lua_ConVar_GetInt(lua_State *l,ConVar *cvar) {Lua::PushInt(l,cvar->GetInt());}
void Lua_ConVar_GetFloat(lua_State *l,ConVar *cvar) {Lua::PushNumber(l,cvar->GetFloat());}
void Lua_ConVar_GetBool(lua_State *l,ConVar *cvar) {lua_pushboolean(l,cvar->GetBool());}
//void Lua_ConVar_SetValue(lua_State *l,ConVar *cvar,std::string value) {cvar->SetValue(value);} // WEAVETODO
void Lua_ConVar_GetFlags(lua_State *l,ConVar *cvar) {Lua::PushInt(l,cvar->GetFlags());}