// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.config_skin;

export import pragma.lua;
export import pragma.gui;

namespace pragma::gui {
	export class DLLCLIENT ConfigSkinWithLua : public ConfigSkin
	{
	public:
		ConfigSkinWithLua();
		using ConfigSkin::Load;
	protected:
		void ApplyStyle(types::WIBase &el, Style &style) const override;
		void Initialize(StyleProperty &rootStyle, const udm::PropertyWrapper &data) override;
	private:
		void ApplyStyleLua(types::WIBase &el, Style &style) const;
	};
};
