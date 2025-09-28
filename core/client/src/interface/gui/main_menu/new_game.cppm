// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:gui.main_menu_new_game;

import :gui.chromium_page;
import :gui.main_menu_base;

export {
	class DLLCLIENT WIMainMenuNewGame : public WIMainMenuBase {
	protected:
		WIHandle m_hMapList;
		WIHandle m_hServerName;
		WIHandle m_hGameMode;
		WIHandle m_hRconPassword;
		WIHandle m_hMaxPlayers;
		CallbackHandle m_cbMapListReload;
		void OnStartGame(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods);
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
