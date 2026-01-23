// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.global;
import :engine;

double Lua::ServerTime(lua::State *l)
{
	auto *state = pragma::get_cengine()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	return game->ServerTime();
}

double Lua::FrameTime(lua::State *l) { return pragma::get_cengine()->GetDeltaFrameTime(); }
