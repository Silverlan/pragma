// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"

#ifdef _MSC_VER
namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};
#endif

#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/mainmenu/wimainmenu_credits.hpp"
#include <wgui/types/witext.h>

import pragma.locale;
import pragma.string.unicode;

WIMainMenuCredits::WIMainMenuCredits() : WIMainMenuBase() {}

WIMainMenuCredits::~WIMainMenuCredits() {}

void WIMainMenuCredits::AddCreditsElement(WIBase &el)
{
	auto hThis = GetHandle();
	el.AddCallback("SetSize", FunctionCallback<void>::Create([hThis]() mutable {
		if(hThis.IsValid() == false)
			return;
		hThis.get()->ScheduleUpdate();
	}));
}

WITexturedRect &WIMainMenuCredits::AddLogo(const std::string &material)
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

WIText &WIMainMenuCredits::AddHeader(const std::string &header, const std::string &headerStyle)
{
	if(headerStyle != "header")
		AddGap(30);
	auto &pText = AddText(header, headerStyle);
	if(headerStyle == "header")
		AddGap(10);
	return pText;
}

WIText &WIMainMenuCredits::AddText(const std::string &header, const std::string &styleClass)
{
	auto *pText = static_cast<WIText *>(WGUI::GetInstance().Create<WIText>(m_creditsContainer.get()));
	pText->SetText(header);
	pText->AddStyleClass(styleClass);
	pText->SetColor(Color::White);
	pText->CenterToParentX();
	AddCreditsElement(*pText);
	return *pText;
}

WIBase &WIMainMenuCredits::AddGap(uint32_t size)
{
	auto *p = WGUI::GetInstance().Create<WIBase>(m_creditsContainer.get());
	p->SetSize(1, size);
	AddCreditsElement(*p);
	return *p;
}

void WIMainMenuCredits::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(pragma::locale::get_text("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}

void WIMainMenuCredits::SetSize(int x, int y)
{
	WIMainMenuBase::SetSize(x, y);
	ScheduleUpdate();
}

void WIMainMenuCredits::DoUpdate()
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

void WIMainMenuCredits::OnVisibilityChanged(bool bVisible)
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
	AddHeader(pragma::locale::get_text("menu_credits"), "header");
	/*AddHeader(pragma::locale::get_text("patrons"));

	for(auto &patron : engine_info::get_patrons())
		AddText(patron,"credits_text");*/

	AddHeader(pragma::locale::get_text("localization"));
	AddText("Shmeerz (" + pragma::locale::get_text("lan_portuguese") + ")", "credits_text");

	AddHeader(pragma::locale::get_text("tools_and_plugins"));
	AddText("Ilya Getsman aka \"RED_EYE\" (Blender plugins)", "credits_text");

	AddHeader(pragma::locale::get_text("powered_by"), "header2");
	AddLogo("third_party/vulkan_logo");
	AddLogo("third_party/fmod_logo");
	AddLogo("third_party/physx_logo");
	AddLogo("third_party/lua_logo").SetWidth(160, true);
	ScheduleUpdate();
}
