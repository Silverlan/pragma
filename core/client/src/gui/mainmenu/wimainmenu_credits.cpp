/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"

namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};

#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/mainmenu/wimainmenu_credits.hpp"
#include <pragma/localization.h>
#include <wgui/types/witext.h>

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
	AddMenuItem(Locale::GetText("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
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
	AddHeader(Locale::GetText("menu_credits"), "header");
	/*AddHeader(Locale::GetText("patrons"));

	for(auto &patron : engine_info::get_patrons())
		AddText(patron,"credits_text");*/

	AddHeader(Locale::GetText("localization"));
	AddText("Shmeerz (" + Locale::GetText("lan_portuguese") + ")", "credits_text");

	AddHeader(Locale::GetText("tools_and_plugins"));
	AddText("Ilya Getsman aka \"RED_EYE\" (Blender plugins)", "credits_text");

	AddHeader(Locale::GetText("powered_by"), "header2");
	AddLogo("third_party/vulkan_logo");
	AddLogo("third_party/fmod_logo");
	AddLogo("third_party/physx_logo");
	AddLogo("third_party/lua_logo").SetWidth(160, true);
	ScheduleUpdate();
}
