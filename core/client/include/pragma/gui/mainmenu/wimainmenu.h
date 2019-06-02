#ifndef __WIMAINMENU_H__
#define __WIMAINMENU_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"
#include <wgui/types/wirect.h>
#include "pragma/gui/wibaseblur.h"
#include <sharedutils/functioncallback.h>

#define WIMENU_ENABLE_PATREON_LOGO 0

class DLLCLIENT WIMainMenu
	: public WIBase,public WIBaseBlur
{
public:
	WIMainMenu();
	virtual ~WIMainMenu() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	void SetContinueMenu();
	void SetNewGameMenu();
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	virtual util::EventReply KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods) override;

	void OpenNewGameMenu();
	void OpenLoadGameMenu();
	void OpenOptionsMenu();
	void OpenModsMenu();
	void OpenCreditsMenu();
	void OpenLoadScreen();
	void OpenMainMenu();
protected:
	WIHandle m_hMain;
	WIHandle m_hNewGame;
	WIHandle m_hLoad;
	WIHandle m_hOptions;
	WIHandle m_hLoadScreen;
	WIHandle m_hBg;
	WIHandle m_hBgSlideShow;
	WIHandle m_hServerBrowser;
	WIHandle m_hVersion;
	WIHandle m_hBuild = {};
	WIHandle m_hVersionAttributes;
#if WIMENU_ENABLE_PATREON_LOGO != 0
	WIHandle m_hPatreonIcon;
#endif
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	WIHandle m_hFMODLogo = {};
#endif
	WIHandle m_hMods;
	WIHandle m_hCredits;
	CallbackHandle m_cbOnGameStart;
	CallbackHandle m_cbOnGameEnd;

	// Menu Tracks
	CallbackHandle m_cbMenuTrack;
	std::vector<std::string> m_menuTracks;
	std::shared_ptr<ALSound> m_menuSound;
	//

	WIHandle m_hActive;
	char m_menuType;
	void SetActiveMenu(WIHandle &hMenu);
	void OnVisibilityChanged(bool bVisible);

	// Blur
	CallbackHandle m_cbBlur;
	double m_tOpen;
	//
	void PlayNextMenuTrack(bool newRound=false);
private:
	CallbackHandle m_cbOnSteamworksInit = {};
	CallbackHandle m_cbOnSteamworksShutdown = {};
};

#endif