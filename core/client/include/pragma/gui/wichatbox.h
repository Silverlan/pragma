/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WICHATBOX_H__
#define __WICHATBOX_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WIChatBox
	: public WIBase
{
public:
	WIChatBox();
	virtual ~WIChatBox() override;
	virtual void Initialize() override;
};

#endif