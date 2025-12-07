// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:gui.main_menu_new_game;

export import :gui.main_menu_base;

export namespace pragma::gui::types {
	class DLLCLIENT WIMainMenuNewGame : public WIMainMenuBase {
	  protected:
		pragma::gui::WIHandle m_hMapList;
		pragma::gui::WIHandle m_hServerName;
		pragma::gui::WIHandle m_hGameMode;
		pragma::gui::WIHandle m_hRconPassword;
		pragma::gui::WIHandle m_hMaxPlayers;
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
