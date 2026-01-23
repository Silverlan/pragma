// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.main_menu_credits;

import pragma.string.unicode;

pragma::gui::types::WIMainMenuCredits::WIMainMenuCredits() : WIMainMenuBase() {}

pragma::gui::types::WIMainMenuCredits::~WIMainMenuCredits() {}

void pragma::gui::types::WIMainMenuCredits::AddCreditsElement(WIBase &el)
{
	auto hThis = GetHandle();
	el.AddCallback("SetSize", FunctionCallback<void>::Create([hThis]() mutable {
		if(hThis.IsValid() == false)
			return;
		hThis.get()->ScheduleUpdate();
	}));
}

pragma::gui::types::WITexturedRect &pragma::gui::types::WIMainMenuCredits::AddLogo(const std::string &material)
{
	AddGap(10);
	auto *el = WGUI::GetInstance().Create<WITexturedRect>(m_creditsContainer.get());
	el->AddStyleClass("credits_logo");
	el->SetMaterial(material);
	el->SizeToTexture();
	el->SetWidth(180, true);
	AddGap(10);
	AddCreditsElement(*el);
	return *el;
}

pragma::gui::types::WIText &pragma::gui::types::WIMainMenuCredits::AddHeader(const std::string &header, const std::string &headerStyle)
{
	if(headerStyle != "header")
		AddGap(30);
	auto &pText = AddText(header, headerStyle);
	if(headerStyle == "header")
		AddGap(10);
	return pText;
}

pragma::gui::types::WIText &pragma::gui::types::WIMainMenuCredits::AddText(const std::string &header, const std::string &styleClass)
{
	auto *pText = static_cast<WIText *>(WGUI::GetInstance().Create<WIText>(m_creditsContainer.get()));
	pText->SetText(header);
	pText->AddStyleClass(styleClass);
	pText->SetColor(colors::White);
	pText->CenterToParentX();
	AddCreditsElement(*pText);
	return *pText;
}

pragma::gui::types::WIBase &pragma::gui::types::WIMainMenuCredits::AddGap(uint32_t size)
{
	auto *p = WGUI::GetInstance().Create<WIBase>(m_creditsContainer.get());
	p->SetSize(1, size);
	AddCreditsElement(*p);
	return *p;
}

void pragma::gui::types::WIMainMenuCredits::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(locale::get_text("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}

void pragma::gui::types::WIMainMenuCredits::SetSize(int x, int y)
{
	WIMainMenuBase::SetSize(x, y);
	ScheduleUpdate();
}

void pragma::gui::types::WIMainMenuCredits::DoUpdate()
{
	WIMainMenuBase::DoUpdate();
	uint32_t y = 0u;
	for(auto &hChild : *m_creditsContainer->GetChildren()) {
		if(hChild.IsValid() == false)
			continue;
		auto x = GetWidth() / 2 - hChild->GetWidth() / 2;
		hChild->SetPos(x, y);
		y += hChild->GetHeight() + 5;
	}
}

void pragma::gui::types::WIMainMenuCredits::OnVisibilityChanged(bool bVisible)
{
	WIMainMenuBase::OnVisibilityChanged(bVisible);

	// Create or destroy contents based on visibility, so we don't keep it around in cache
	if(bVisible == false) {
		if(m_creditsContainer.IsValid())
			m_creditsContainer->Remove();
		return;
	}
	if(m_creditsContainer.IsValid())
		return;
	m_creditsContainer = CreateChild<WIBase>();
	m_creditsContainer->SetAutoAlignToParent(true);

	AddGap(120);
	AddHeader(locale::get_text("menu_credits"), "header");
	/*AddHeader(pragma::locale::get_text("patrons"));

	for(auto &patron : engine_info::get_patrons())
		AddText(patron,"credits_text");*/

	AddHeader(locale::get_text("localization"));
	AddText("Shmeerz (" + locale::get_text("lan_portuguese") + ")", "credits_text");

	AddHeader(locale::get_text("tools_and_plugins"));
	AddText("Ilya Getsman aka \"RED_EYE\" (Blender plugins)", "credits_text");

	AddHeader(locale::get_text("powered_by"), "header2");
	AddLogo("third_party/vulkan_logo");
	AddLogo("third_party/fmod_logo");
	AddLogo("third_party/physx_logo");
	AddLogo("third_party/lua_logo").SetWidth(160, true);
	ScheduleUpdate();
}
