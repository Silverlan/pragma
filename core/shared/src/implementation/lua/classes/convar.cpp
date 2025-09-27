// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/classes/lconvar.h"
#include <pragma/engine.h>
#include "luasystem.h"
#include <pragma/game/game.h>
#include <mathutil/umath.h>
#include <pragma/console/command_options.hpp>

module pragma.shared;

import :scripting.lua.classes.convar;

ConVar *Lua::console::CreateConVar(lua_State *l, const std::string &cmd, ::udm::Type type, Lua::udm_type def, ConVarFlags flags, const std::string &help)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return 0;
	auto cvar = udm::visit(type, [&def, flags, &help](auto tag) {
		using T = typename decltype(tag)::type;
		constexpr auto type = udm::type_to_enum<T>();
		if constexpr(type == udm::Type::Element || udm::is_array_type(type))
			return std::shared_ptr<ConVar> {nullptr};
		else {
			auto v = luabind::object_cast<T>(def);
			return ConVar::Create<T>(v, flags, help);
		}
	});
	if(!cvar)
		return 0;
	return state->RegisterConVar(cmd, cvar);
}

void Lua::console::CreateConCommand(lua_State *l, const std::string &name, const Lua::func<void, pragma::BasePlayerComponent, float, Lua::variadic<std::string>> &function, ConVarFlags flags, const std::string &help)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	state->CreateConCommand(name, function, flags, help);
}
void Lua::console::CreateConCommand(lua_State *l, const std::string &name, const Lua::func<void, pragma::BasePlayerComponent, float, Lua::variadic<std::string>> &function, ConVarFlags flags)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	state->CreateConCommand(name, function, flags);
}
void Lua::console::CreateConCommand(lua_State *l, const std::string &name, const Lua::func<void, pragma::BasePlayerComponent, float, Lua::variadic<std::string>> &function, const std::string &help)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	state->CreateConCommand(name, function, ConVarFlags::None, help);
}

ConVar *Lua::console::GetConVar(lua_State *l, const std::string &name)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return nullptr;
	auto *cv = state->GetConVar(name);
	if(cv == nullptr)
		return nullptr;
	if(cv->GetType() != ConType::Var)
		return nullptr;
	return static_cast<ConVar *>(cv);
}

static ConVar *get_con_var(lua_State *l, const std::string &conVar)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return nullptr;
	auto *cv = state->GetConVar(conVar);
	if(cv == nullptr || cv->GetType() != ConType::Var)
		return nullptr;
	return static_cast<ConVar *>(cv);
}

int32_t Lua::console::GetConVarInt(lua_State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetInt() : 0;
}

float Lua::console::GetConVarFloat(lua_State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetFloat() : 0.f;
}

std::string Lua::console::GetConVarString(lua_State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetString() : "";
}

bool Lua::console::GetConVarBool(lua_State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetBool() : false;
}

ConVarFlags Lua::console::GetConVarFlags(lua_State *l, const std::string &conVar)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return ConVarFlags::None;
	auto *cv = state->GetConVar(conVar);
	if(cv == nullptr)
		return ConVarFlags::None;
	return cv->GetFlags();
}

int Lua::console::Run(lua_State *l)
{
	int argc = lua_gettop(l);
	std::string cmd = luaL_checkstring(l, 1);
	int i = 2;
	while(argc >= i) {
		std::string arg = luaL_checkstring(l, i);
		cmd += " \"" + arg + "\"";
		i++;
	}
	//Engine::Get()->ConsoleInput(cmd.c_str());
	Engine::Get()->ProcessConsoleInput(cmd);
	return 0;
}

int Lua::console::AddChangeCallback(lua_State *l)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	if(game == nullptr)
		return 0;
	auto cvar = Lua::CheckString(l, 1);
	Lua::CheckFunction(l, 2);
	auto fc = luabind::object(luabind::from_stack(l, 2));
	auto cb = game->AddConVarCallback(cvar, fc);
	if(!cb.IsValid())
		return 0;
	Lua::Push(l, cb);
	return 1;
}

void Lua::console::register_override(lua_State *l, const std::string &src, const std::string &dst)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	state->SetConsoleCommandOverride(src, dst);
}
void Lua::console::clear_override(lua_State *l, const std::string &src)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	state->ClearConsoleCommandOverride(src);
}
int Lua::console::parse_command_arguments(lua_State *l)
{
	int32_t tArgs = 1;
	Lua::CheckTable(l, tArgs);
	std::vector<std::string> args {};
	auto numArgs = Lua::GetObjectLength(l, tArgs);
	args.reserve(numArgs);
	for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
		Lua::PushInt(l, i + 1);       /* 1 */
		Lua::GetTableValue(l, tArgs); /* 1 */
		args.push_back(Lua::CheckString(l, -1));

		Lua::Pop(l, 1); /* 0 */
	}

	std::unordered_map<std::string, pragma::console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(args, commandOptions);

	auto t = Lua::CreateTable(l);
	for(auto &pair : commandOptions) {
		Lua::PushString(l, pair.first);  /* 1 */
		auto tCmd = Lua::CreateTable(l); /* 2 */
		auto &cmd = pair.second;
		int32_t argIdx = 1;
		for(auto &str : cmd.parameters) {
			Lua::PushInt(l, argIdx++);   /* 3 */
			Lua::PushString(l, str);     /* 4 */
			Lua::SetTableValue(l, tCmd); /* 2 */
		}
		Lua::SetTableValue(l, t); /* 0 */
	}
	return 1;
}
