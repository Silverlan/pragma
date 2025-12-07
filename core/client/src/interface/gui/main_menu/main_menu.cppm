// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:gui.main_menu;

export import pragma.gui;
export import pragma.shared;

export namespace pragma::gui::types {
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
		virtual util::EventReply KeyboardCallback(pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;

		void OpenNewGameMenu();
		void OpenLoadGameMenu();
		void OpenOptionsMenu();
		void OpenModsMenu();
		void OpenLoadScreen();
		void OpenMainMenu();
	  protected:
		pragma::gui::WIHandle m_hMain;
		pragma::gui::WIHandle m_hNewGame;
		pragma::gui::WIHandle m_hLoad;
		pragma::gui::WIHandle m_hOptions;
		pragma::gui::WIHandle m_hLoadScreen;
		pragma::gui::WIHandle m_hBg;
		pragma::gui::WIHandle m_hBgSlideShow;
		pragma::gui::WIHandle m_hServerBrowser;
		pragma::gui::WIHandle m_hVersion;
		pragma::gui::WIHandle m_hPragmaLogo = {};
		pragma::gui::WIHandle m_hBuild = {};
		pragma::gui::WIHandle m_logoContainer = {};
		pragma::gui::WIHandle m_hVersionAttributes;
		pragma::gui::WIHandle m_hRenderAPI;
		pragma::gui::WIHandle m_hMods;
		pragma::gui::WIHandle m_hCredits;
		CallbackHandle m_cbOnGameStart;
		CallbackHandle m_cbOnGameEnd;

		// Menu Tracks
		CallbackHandle m_cbMenuTrack;
		std::vector<std::string> m_menuTracks;
		std::shared_ptr<pragma::audio::ALSound> m_menuSound;
		//

		pragma::gui::WIHandle m_hActive;
		char m_menuType;
		void SetActiveMenu(WIHandle &hMenu);
		virtual void OnVisibilityChanged(bool bVisible) override;

		// Blur
		CallbackHandle m_cbBlur;
		double m_tOpen;
		//
		void PlayNextMenuTrack(bool newRound = false);
	  private:
		CallbackHandle m_cbOnSteamworksInit = {};
		CallbackHandle m_cbOnSteamworksShutdown = {};
	};
};
