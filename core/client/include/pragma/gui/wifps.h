/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIFPS_H__
#define __WIFPS_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WIFPS
	: public WIBase
{
private:
	WIHandle m_text;
	uint32_t m_fpsLast;
	double m_tLastUpdate;
public:
	WIFPS();
	virtual ~WIFPS() override;
	virtual void Initialize() override;
	virtual void Think() override;
};

#endif