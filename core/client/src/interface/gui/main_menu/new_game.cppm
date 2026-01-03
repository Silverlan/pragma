// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.main_menu_new_game;

export import :gui.main_menu_base;

export namespace pragma::gui::types {
	class DLLCLIENT WIMainMenuNewGame : public WIMainMenuBase {
	  protected:
		WIHandle m_hMapList;
		WIHandle m_hServerName;
		WIHandle m_hGameMode;
		WIHandle m_hRconPassword;
		WIHandle m_hMaxPlayers;
		CallbackHandle m_cbMapListReload;
		void OnStartGame(platform::MouseButton button, platform::KeyState state, platform::Modifier mods);
		void InitializeGameSettings();
		virtual void InitializeOptionsList(WIOptionsList *pList) override;
		using WIMainMenuBase::InitializeOptionsList;
	  public:
		WIMainMenuNewGame();
		virtual ~WIMainMenuNewGame() override;
		virtual void Initialize() override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
		void ReloadMapList();
	};
};
