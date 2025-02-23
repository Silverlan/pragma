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
#include "pragma/gui/wiloadscreen.h"
#include "pragma/gui/wiprogressbar.h"
#include <wgui/types/witext.h>

import pragma.locale;

extern DLLCLIENT CEngine *c_engine;
extern ClientState *client;
WILoadScreen::WILoadScreen() : WIMainMenuBase() {}

WILoadScreen::~WILoadScreen() {}

void WILoadScreen::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(pragma::locale::get_text("cancel"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
	m_hText = CreateChild<WIText>();
	auto *pText = static_cast<WIText *>(m_hText.get());
	pText->SetText("LOADING...");
	pText->SetColor(1.f, 1.f, 1.f, 1.f);
	pText->SizeToContents();
	pText->SetAutoCenterToParentX(true);

	m_hProgress = CreateChild<WIProgressBar>();
	auto *pProgressBar = static_cast<WIProgressBar *>(m_hProgress.get());
	pProgressBar->SetAutoCenterToParentX(true);
}

void WILoadScreen::SetSize(int x, int y)
{
	WIMainMenuBase::SetSize(x, y);
	auto yOffset = 0;
	if(m_hText.IsValid()) {
		yOffset = CInt32(CFloat(y) * 0.6f);
		auto *pText = static_cast<WIText *>(m_hText.get());
		pText->SetY(yOffset);
		yOffset += pText->GetHeight();
	}
	if(m_hProgress.IsValid()) {
		auto *pProgressBar = static_cast<WIProgressBar *>(m_hProgress.get());
		pProgressBar->SetProgress(0.5f);
		pProgressBar->SetY(yOffset);
		pProgressBar->SetWidth(512);
	}
}
