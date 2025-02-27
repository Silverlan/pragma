/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
