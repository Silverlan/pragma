// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "luasystem.h"
#include "pragma/ai/c_lai.hpp"
#include <pragma/lua/libraries/lai.hpp>
#include <luainterface.hpp>

void Lua::ai::client::register_library(Lua::Interface &lua)
{
	auto &modAi = lua.RegisterLibrary("ai");
	Lua::ai::register_library(lua);
}
