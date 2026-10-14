// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.json_skin;

export import pragma.lua;
export import pragma.gui;

namespace pragma::gui {
	export class DLLCLIENT JsonSkinWithLua : public JsonSkin
	{
	public:
		JsonSkinWithLua();
		using JsonSkin::Load;
	protected:
		void ApplyStyle(types::WIBase &el, Style &style) const override;
		void Load(StyleProperty &styleProp) override;
	private:
		void ApplyStyleLua(types::WIBase &el, Style &style) const;
	};
};
