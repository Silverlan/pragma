// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/util.hpp"
#include "pragma/c_engine.h"

module pragma.client.gui;

import pragma.client.client_state;

bool load_skin(const std::string &skinName)
{
	auto luaPath = Lua::find_script_file("gui/skins/" + skinName);
	return luaPath && pragma::get_client_game()->ExecuteLuaFile(*luaPath, pragma::get_client_state()->GetGUILuaState());
}
