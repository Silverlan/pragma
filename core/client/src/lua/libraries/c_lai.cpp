/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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