// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/util.hpp"

#include "pragma/lua/luaapi.h"

#include "stdafx_client.h"

module pragma.client;

import :gui;
import :client_state;
import :engine;
import :game;

bool load_skin(const std::string &skinName)
{
	auto luaPath = Lua::find_script_file("gui/skins/" + skinName);
	return luaPath && pragma::get_client_game()->ExecuteLuaFile(*luaPath, pragma::get_client_state()->GetGUILuaState());
}
