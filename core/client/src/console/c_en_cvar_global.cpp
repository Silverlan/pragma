// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_cengine.h"
#include <fsys/filesystem.h>
#include "pragma/c_engine.h"
#include "pragma/console/c_en_cvar_global.h"
#include <sharedutils/util_string.h>
#include <pragma/lua/libraries/lutil.hpp>
#include <pragma/clientstate/clientstate.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;

#if LUA_ENABLE_RUN_GUI == 1
void CMD_lua_run_gui(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::cwar << "No argument given to execute!" << Con::endl;
		return;
	}
	std::string lua = argv[0];
	for(auto i = 1; i < argv.size(); i++) {
		lua += " ";
		lua += argv[i];
	}
	auto *l = static_cast<ClientState *>(state)->GetGUILuaState();
	Lua::Execute(l, [l, &lua](int (*traceback)(lua_State *)) { return Lua::RunString(l, lua, "lua_run_gui", traceback); });
}
#endif

DLLCLIENT void CMD_lua_run_cl(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::cwar << "No argument given to execute!" << Con::endl;
		return;
	}
	if(!state->IsGameActive() || state->GetGameState() == nullptr) {
		Con::cwar << "No game is active! Lua code cannot be executed without an active game!" << Con::endl;
		return;
	}

	std::string lua = argv[0];
	for(auto i = 1; i < argv.size(); i++) {
		lua += " ";
		lua += argv[i];
	}
	state->GetGameState()->RunLua(lua);
}

DLLCLIENT void CMD_lua_reload_entity(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	ClientState *cstate = static_cast<ClientState *>(state);
	CGame *game = cstate->GetGameState();
	if(game == NULL)
		return;
	game->LoadLuaEntity("entities", argv[0]);
}

DLLCLIENT void CMD_lua_reload_weapon(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	ClientState *cstate = static_cast<ClientState *>(state);
	CGame *game = cstate->GetGameState();
	if(game == NULL)
		return;
	game->LoadLuaEntity("weapons", argv[0]);
}

DLLCLIENT void CMD_lua_reload_entities(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	NetworkState *server = c_engine->GetServerNetworkState();
	Game *sgame = server->GetGameState();
	if(sgame != NULL)
		sgame->LoadLuaEntities("entities");
	NetworkState *client = c_engine->GetClientState();
	Game *cgame = client->GetGameState();
	if(cgame != NULL)
		cgame->LoadLuaEntities("entities");
}

DLLCLIENT void CMD_lua_reload_weapons(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	NetworkState *server = c_engine->GetServerNetworkState();
	Game *sgame = server->GetGameState();
	if(sgame != NULL)
		sgame->LoadLuaEntities("weapons");
	NetworkState *client = c_engine->GetClientState();
	Game *cgame = client->GetGameState();
	if(cgame != NULL)
		cgame->LoadLuaEntities("weapons");
}
