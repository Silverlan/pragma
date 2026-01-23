// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.ai;
void Lua::ai::client::register_library(Interface &lua)
{
	auto &modAi = lua.RegisterLibrary("ai");
	ai::register_library(lua);
}
