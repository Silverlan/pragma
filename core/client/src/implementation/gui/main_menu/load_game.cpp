// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.main_menu_load_game;
import :client_state;
import :engine;

pragma::gui::types::WIMainMenuLoadGame::WIMainMenuLoadGame() : WIMainMenuBase() {}

pragma::gui::types::WIMainMenuLoadGame::~WIMainMenuLoadGame() {}

void pragma::gui::types::WIMainMenuLoadGame::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(locale::get_text("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}
