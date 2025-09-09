// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "luasystem.h"

module pragma.client.scripting.lua.libraries.global;

extern CEngine *c_engine;

double Lua::ServerTime(lua_State *l)
{
	NetworkState *state = c_engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	return game->ServerTime();
}

double Lua::FrameTime(lua_State *l) { return c_engine->GetDeltaFrameTime(); }
