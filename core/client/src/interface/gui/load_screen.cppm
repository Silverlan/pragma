// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:gui.load_screen;

export import :gui.main_menu_base;

export namespace pragma::gui::types {
	class DLLCLIENT WILoadScreen : public WIMainMenuBase {
	  private:
		pragma::gui::WIHandle m_hText;
		pragma::gui::WIHandle m_hProgress;
	  public:
		WILoadScreen();
		virtual ~WILoadScreen() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		using WIMainMenuBase::SetSize;
	};
};
