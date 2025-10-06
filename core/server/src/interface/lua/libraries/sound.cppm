// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.server.scripting.lua.libraries.sound;

export namespace Lua {
	namespace sound {
		namespace Server {
			DLLSERVER int create(lua_State *l);
		};
	};
};
