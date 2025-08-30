// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/converters/gui_element_converter.hpp"

export module pragma.client.gui:main_menu_load_game;

import :chromium_page;
import :main_menu_base;

export {
	class DLLCLIENT WIMainMenuLoadGame : public WIMainMenuBase {
	public:
		WIMainMenuLoadGame();
		virtual ~WIMainMenuLoadGame() override;
		virtual void Initialize() override;
	};
};
