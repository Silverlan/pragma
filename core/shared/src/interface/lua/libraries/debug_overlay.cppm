// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.debug_overlay;

export import pragma.lua;

export {
	DLLNETWORK int Lua_debugoverlay_DrawLine(lua::State *l);
	DLLNETWORK int Lua_debugoverlay_DrawBox(lua::State *l);
};
