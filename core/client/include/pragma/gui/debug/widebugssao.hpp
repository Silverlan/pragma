// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
