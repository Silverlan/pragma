// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:scripting.lua.libraries.global;
export namespace Lua {
	DLLCLIENT double ServerTime(lua_State *l);
	DLLCLIENT double FrameTime(lua_State *l);

	DLLCLIENT void register_shared_client_state(lua_State *l);
};
