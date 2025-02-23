/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/mainmenu/wimainmenu_loadgame.h"

import pragma.locale;

extern DLLCLIENT CEngine *c_engine;
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
