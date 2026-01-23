// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.load_screen;
import :gui.progress_bar;

import :client_state;
import :engine;

pragma::gui::types::WILoadScreen::WILoadScreen() : WIMainMenuBase() {}

pragma::gui::types::WILoadScreen::~WILoadScreen() {}

void pragma::gui::types::WILoadScreen::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(locale::get_text("cancel"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
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

void pragma::gui::types::WILoadScreen::SetSize(int x, int y)
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
