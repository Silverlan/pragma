// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.icon;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIIcon : public WITexturedRect {
	  public:
		WIIcon();
		virtual ~WIIcon() override;
		virtual void Initialize() override;
		void SetClipping(uint32_t xStart, uint32_t yStart, uint32_t width, uint32_t height);
	};
};
