// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/lua/converters/gui_element_converter.hpp"

module pragma.client.gui;

import :main_menu_load_game;
import pragma.client.client_state;
import pragma.locale;

extern CEngine *c_engine;
extern ClientState *client;
WIMainMenuLoadGame::WIMainMenuLoadGame() : WIMainMenuBase() {}

WIMainMenuLoadGame::~WIMainMenuLoadGame() {}

void WIMainMenuLoadGame::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(pragma::locale::get_text("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}
