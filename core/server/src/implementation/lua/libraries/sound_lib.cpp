// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.scripting.lua.libraries.sound;

import pragma.server.server_state;

int Lua::sound::Server::create(lua_State *l)
{
	auto bShared = true;
	if(Lua::IsSet(l, 4))
		bShared = Lua::CheckBool(l, 4);
	return Lua::sound::create(l, [bShared](NetworkState *nw, const std::string &name, ALSoundType type, ALCreateFlags flags) -> std::shared_ptr<::ALSound> { return static_cast<ServerState *>(nw)->CreateSound(name, type, flags); });
}
