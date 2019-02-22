#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/wiloadscreen.h"
#include "pragma/gui/wiprogressbar.h"
#include <wgui/types/witext.h>
#include "pragma/localization.h"

extern DLLCENGINE CEngine *c_engine;
extern ClientState *client;
WILoadScreen::WILoadScreen()
	: WIMainMenuBase()
{}

WILoadScreen::~WILoadScreen()
{}

void WILoadScreen::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(Locale::GetText("cancel"),FunctionCallback<void,WIMainMenuElement*>::Create([this](WIMainMenuElement*) {
		auto *mainMenu = dynamic_cast<WIMainMenu*>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
	m_hText = CreateChild<WIText>();
	auto *pText = m_hText.get<WIText>();
	pText->SetText("LOADING...");
	pText->SetColor(1.f,1.f,1.f,1.f);
	pText->SizeToContents();
	pText->SetAutoCenterToParentX(true);

	m_hProgress = CreateChild<WIProgressBar>();
	auto *pProgressBar = m_hProgress.get<WIProgressBar>();
	pProgressBar->SetAutoCenterToParentX(true);
}

void WILoadScreen::SetSize(int x,int y)
{
	WIMainMenuBase::SetSize(x,y);
	auto yOffset = 0;
	if(m_hText.IsValid())
	{
		yOffset = CInt32(CFloat(y) *0.6f);
		auto *pText = m_hText.get<WIText>();
		pText->SetY(yOffset);
		yOffset += pText->GetHeight();
	}
	if(m_hProgress.IsValid())
	{
		auto *pProgressBar = m_hProgress.get<WIProgressBar>();
		pProgressBar->SetProgress(0.5f);
		pProgressBar->SetY(yOffset);
		pProgressBar->SetWidth(512);
	}
}