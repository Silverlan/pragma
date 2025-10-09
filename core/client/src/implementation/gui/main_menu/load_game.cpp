// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "sharedutils/functioncallback.h"

#include "stdafx_client.h"

module pragma.client;

import :gui.main_menu_load_game;
import :client_state;
import :engine;
import pragma.locale;

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
