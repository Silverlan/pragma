/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WICHECKBOX_H__
#define __WICHECKBOX_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

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

#endif
