// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
		virtual util::EventReply KeyboardCallback(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) override;

		void OpenNewGameMenu();
		void OpenLoadGameMenu();
		void OpenOptionsMenu();
		void OpenModsMenu();
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
		WIHandle m_hMods;
		WIHandle m_hCredits;
		CallbackHandle m_cbOnGameStart;
		CallbackHandle m_cbOnGameEnd;

		// Menu Tracks
		CallbackHandle m_cbMenuTrack;
		std::vector<std::string> m_menuTracks;
		std::shared_ptr<audio::ALSound> m_menuSound;
		//

		WIHandle m_hActive;
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
