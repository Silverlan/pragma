// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"


module pragma.client;


import :scripting.lua.libraries.ai;
void Lua::ai::client::register_library(Lua::Interface &lua)
{
	auto &modAi = lua.RegisterLibrary("ai");
	Lua::ai::register_library(lua);
}
