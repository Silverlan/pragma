// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.debug_ssao;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIDebugSSAO : public WITexturedRect {
	  public:
		WIDebugSSAO();
		void SetUseBlurredSSAOImage(bool b);
	  private:
		virtual void DoUpdate() override;
		bool m_bUseBlurVariant = false;
	};
};
