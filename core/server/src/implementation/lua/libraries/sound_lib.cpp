// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.scripting.lua.libraries.sound;

import pragma.server.server_state;

std::shared_ptr<::ALSound> Lua::sound::Server::create(lua_State *l, const std::string &snd, ALSoundType type, ALCreateFlags flags)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	auto pAl = state->CreateSound(snd, type, flags);
	if(pAl == nullptr)
		return nullptr;
	pAl->SetType(type);
	return pAl;
}
