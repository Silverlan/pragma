#include "stdafx_client.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/mainmenu/wimainmenu_credits.hpp"
#include <pragma/localization.h>

WIMainMenuCredits::WIMainMenuCredits()
	: WIMainMenuBase(),WIChromiumPage(this)
{}

WIMainMenuCredits::~WIMainMenuCredits()
{}

void WIMainMenuCredits::OnVisibilityChanged(bool bVisible) {WIChromiumPage::OnVisibilityChanged(bVisible);}
void WIMainMenuCredits::OnFirstEntered()
{
	SetInitialURL(engine_info::get_modding_hub_url() +"html/game/credits.php");
	WIChromiumPage::OnFirstEntered();
}

void WIMainMenuCredits::InitializeWebView(WIBase *el)
{
	WIChromiumPage::InitializeWebView(el);
	el->SetSize(1024,768);
	el->SetPos(100,200);
}

void WIMainMenuCredits::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(Locale::GetText("back"),FunctionCallback<void,WIMainMenuElement*>::Create([this](WIMainMenuElement*) {
		auto *mainMenu = dynamic_cast<WIMainMenu*>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}

