/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIMAINMENU_CREDITS_HPP__
#define __WIMAINMENU_CREDITS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"

class WITexturedRect;
class WIText;
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

#endif
