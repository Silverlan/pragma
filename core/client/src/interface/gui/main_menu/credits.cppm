// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"
#include <wgui/types/wirect.h>
#include <wgui/types/witext.h>

export module pragma.client.gui:main_menu_credits;

import :chromium_page;

export {
	class DLLCLIENT WIMainMenuCredits : public WIMainMenuBase {
	public:
		WIMainMenuCredits();
		virtual ~WIMainMenuCredits() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
	protected:
		virtual void DoUpdate() override;
		virtual void OnVisibilityChanged(bool bVisible) override;
		WITexturedRect &AddLogo(const std::string &material);
		WIText &AddHeader(const std::string &header, const std::string &headerStyle = "header2");
		WIText &AddText(const std::string &header, const std::string &styleClass);
		WIBase &AddGap(uint32_t size);
		void AddCreditsElement(WIBase &el);
		WIHandle m_creditsContainer = {};
	};
};
