// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_client.h"
#include "luasystem.h"
#include <luainterface.hpp>

module pragma.client;


import :scripting.lua.libraries.ai;
void Lua::ai::client::register_library(Lua::Interface &lua)
{
	auto &modAi = lua.RegisterLibrary("ai");
	Lua::ai::register_library(lua);
}
