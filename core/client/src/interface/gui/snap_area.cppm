// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

export module pragma.client:gui.snap_area;

export class DLLCLIENT WISnapArea : public WIBase {
  public:
	WISnapArea();
	virtual void Initialize() override;
	WIBase *GetTriggerArea();
  private:
	WIHandle m_hTriggerArea = {};
};
