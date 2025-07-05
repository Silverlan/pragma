// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_lglobal.h"
#include "luasystem.h"

extern DLLCLIENT CEngine *c_engine;

double Lua::ServerTime(lua_State *l)
{
	NetworkState *state = c_engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	return game->ServerTime();
}

double Lua::FrameTime(lua_State *l) { return c_engine->GetDeltaFrameTime(); }
