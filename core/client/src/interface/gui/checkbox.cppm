// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:gui.checkbox;

export import pragma.gui;

export namespace pragma::gui {
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
		virtual util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
		virtual void SetSize(int x, int y) override;
	};
};
