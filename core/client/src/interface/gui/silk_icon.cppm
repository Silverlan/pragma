// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.silk_icon;

export import :gui.icon;

export namespace pragma::gui::types {
	class DLLCLIENT WISilkIcon : public WIIcon {
	  public:
		WISilkIcon();
		virtual ~WISilkIcon() override;
		virtual void Initialize() override;
		void SetIcon(std::string icon);
	};
};
