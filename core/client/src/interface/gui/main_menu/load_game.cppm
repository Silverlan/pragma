// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.main_menu_load_game;

export import :gui.main_menu_base;

export namespace pragma::gui::types {
	class DLLCLIENT WIMainMenuLoadGame : public WIMainMenuBase {
	  public:
		WIMainMenuLoadGame();
		virtual ~WIMainMenuLoadGame() override;
		virtual void Initialize() override;
	};
};
