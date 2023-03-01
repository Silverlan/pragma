/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIDEBUGSSAO_HPP__
#define __WIDEBUGSSAO_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIDebugSSAO : public WITexturedRect {
  public:
	WIDebugSSAO();
	void SetUseBlurredSSAOImage(bool b);
  private:
	virtual void DoUpdate() override;
	bool m_bUseBlurVariant = false;
};

#endif
