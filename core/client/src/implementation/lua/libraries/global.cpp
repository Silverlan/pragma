// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_client.h"
#include "luasystem.h"

module pragma.client;


import :scripting.lua.libraries.global;
import :engine;


double Lua::ServerTime(lua_State *l)
{
	NetworkState *state = pragma::get_cengine()->GetNetworkState(l);
	Game *game = state->GetGameState();
	return game->ServerTime();
}

double Lua::FrameTime(lua_State *l) { return pragma::get_cengine()->GetDeltaFrameTime(); }
