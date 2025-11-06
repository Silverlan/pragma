// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:scripting.lua.libraries.global;
export namespace Lua {
	DLLCLIENT double ServerTime(lua::State *l);
	DLLCLIENT double FrameTime(lua::State *l);

	DLLCLIENT void register_shared_client_state(lua::State *l);
};
