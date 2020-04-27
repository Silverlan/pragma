/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WISNAPAREA_HPP__
#define __WISNAPAREA_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WISnapArea
	: public WIBase
{
public:
	WISnapArea();
	virtual void Initialize() override;
	WIBase *GetTriggerArea();
private:
	WIHandle m_hTriggerArea = {};
};

#endif