// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"

export module pragma.shared:scripting.lua.libraries.os;

export namespace Lua {
	namespace os {
		DLLNETWORK int64_t time_since_epoch(lua_State *l);
	};
};
