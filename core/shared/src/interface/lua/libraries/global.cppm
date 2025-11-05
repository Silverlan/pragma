// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.global;

export import pragma.lua;

export DLLNETWORK int Lua_RegisterCSLuaFile(lua::State *l);
