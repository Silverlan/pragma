// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_cengine.h"
#include <fsys/filesystem.h>
#include "pragma/console/c_en_cvar_global.h"
#include <sharedutils/util_string.h>
#include <pragma/lua/libraries/lutil.hpp>

import pragma.client.client_state;
import pragma.client.engine;

extern CEngine *c_engine;
extern ClientState *client;

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
