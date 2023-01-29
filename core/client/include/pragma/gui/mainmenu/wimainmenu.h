/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIMAINMENU_H__
#define __WIMAINMENU_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"
#include <wgui/types/wirect.h>
#include <sharedutils/functioncallback.h>

#define WIMENU_ENABLE_PATREON_LOGO 0
#define WIMENU_ENABLE_FMOD_LOGO 0 // (ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD)
#define WIMENU_ENABLE_CREDITS_MENU 0

class DLLCLIENT WIMainMenu : public WIBase {
  public:
	WIMainMenu();
	virtual ~WIMainMenu() override;
	virtual void Initialize() override;
	virtual void SetSize(int x, int y) override;
	void SetContinueMenu();
	void SetNewGameMenu();
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	virtual util::EventReply KeyboardCallback(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods) override;

	void OpenNewGameMenu();
	void OpenLoadGameMenu();
	void OpenOptionsMenu();
	void OpenModsMenu();
#if WIMENU_ENABLE_CREDITS_MENU != 0
	void OpenCreditsMenu();
#endif
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
	WIHandle m_hPragmaLogo = {};
	WIHandle m_hBuild = {};
	WIHandle m_logoContainer = {};
	WIHandle m_hVersionAttributes;
	WIHandle m_hRenderAPI;
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
	void PlayNextMenuTrack(bool newRound = false);
  private:
	CallbackHandle m_cbOnSteamworksInit = {};
	CallbackHandle m_cbOnSteamworksShutdown = {};
};

#endif
