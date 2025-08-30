// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_listener.h"
#include <pragma/lua/ldefinitions.h>

export module pragma.client.scripting.lua.classes.listener;

export {
    DLLCLIENT void Lua_Listener_GetGain(lua_State *l, ListenerHandle &hEnt);
    DLLCLIENT void Lua_Listener_SetGain(lua_State *l, ListenerHandle &hEnt, float gain);
};
