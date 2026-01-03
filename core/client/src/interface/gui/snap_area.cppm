// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.snap_area;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WISnapArea : public WIBase {
	  public:
		WISnapArea();
		virtual void Initialize() override;
		WIBase *GetTriggerArea();
	  private:
		WIHandle m_hTriggerArea = {};
	};
}
