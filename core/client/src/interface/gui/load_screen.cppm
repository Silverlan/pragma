// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.load_screen;

export import :gui.main_menu_base;

export namespace pragma::gui::types {
	class DLLCLIENT WILoadScreen : public WIMainMenuBase {
	  public:
		WILoadScreen();
		virtual ~WILoadScreen() override;
		virtual void Initialize() override;
		using WIMainMenuBase::SetSize;
	  private:
	    virtual void OnSizeChanged(const Vector2i &oldSize, ChangeSource changeSource) override;
		WIHandle m_hText;
		WIHandle m_hProgress;
	};
};
