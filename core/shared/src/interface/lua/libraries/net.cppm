// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

export module pragma.client:scripting.lua.libraries.net;

export namespace Lua {
	namespace net {
		DLLNETWORK void RegisterLibraryEnums(lua_State *l);
		DLLNETWORK pragma::NetEventId register_event(lua_State *l, const std::string &name);
	};
};
