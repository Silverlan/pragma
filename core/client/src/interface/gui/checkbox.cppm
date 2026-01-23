// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.checkbox;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WICheckbox : public WIRect {
	  protected:
		WIHandle m_hOutline;
		WIHandle m_hTick;
		bool m_bChecked;
	  public:
		WICheckbox();
		virtual ~WICheckbox() override;
		virtual void Initialize() override;
		void SetChecked(bool bChecked);
		bool IsChecked();
		void Toggle();
		virtual util::EventReply MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods) override;
		virtual void SetSize(int x, int y) override;
	};
};
