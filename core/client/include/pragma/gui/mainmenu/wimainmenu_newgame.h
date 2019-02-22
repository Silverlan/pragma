#ifndef __WIMAINMENU_NEWGAME_H__
#define __WIMAINMENU_NEWGAME_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"

class DLLCLIENT WIMainMenuNewGame
	: public WIMainMenuBase
{
protected:
	WIHandle m_hMapList;
	WIHandle m_hServerName;
	WIHandle m_hGameMode;
	WIHandle m_hRconPassword;
	WIHandle m_hMaxPlayers;
	CallbackHandle m_cbMapListReload;
	void OnStartGame(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods);
	void InitializeGameSettings();
	virtual void InitializeOptionsList(WIOptionsList *pList) override;
	using WIMainMenuBase::InitializeOptionsList;
public:
	WIMainMenuNewGame();
	virtual ~WIMainMenuNewGame() override;
	virtual void Initialize() override;
	void ReloadMapList();
};


#endif