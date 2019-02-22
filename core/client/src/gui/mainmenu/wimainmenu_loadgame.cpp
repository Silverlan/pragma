#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/mainmenu/wimainmenu_loadgame.h"
#include "pragma/localization.h"

extern DLLCENGINE CEngine *c_engine;
extern ClientState *client;
WIMainMenuLoadGame::WIMainMenuLoadGame()
	: WIMainMenuBase()
{}

WIMainMenuLoadGame::~WIMainMenuLoadGame()
{}

void WIMainMenuLoadGame::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(Locale::GetText("back"),FunctionCallback<void,WIMainMenuElement*>::Create([this](WIMainMenuElement*) {
		auto *mainMenu = dynamic_cast<WIMainMenu*>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}