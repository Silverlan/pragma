// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"

export module pragma.shared:scripting.lua.libraries.debug_overlay;

export {
    DLLNETWORK int Lua_debugoverlay_DrawLine(lua_State *l);
    DLLNETWORK int Lua_debugoverlay_DrawBox(lua_State *l);
};
