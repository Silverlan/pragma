/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lsound.h"

int Lua::sound::Server::create(lua_State *l)
{
	auto bShared = true;
	if(Lua::IsSet(l, 4))
		bShared = Lua::CheckBool(l, 4);
	return Lua::sound::create(l, [bShared](NetworkState *nw, const std::string &name, ALSoundType type, ALCreateFlags flags) -> std::shared_ptr<ALSound> { return static_cast<ServerState *>(nw)->CreateSound(name, type, flags); });
}
