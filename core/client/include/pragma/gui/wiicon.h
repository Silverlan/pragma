/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WIICON_H__
#define __WIICON_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIIcon
	: public WITexturedRect
{
public:
	WIIcon();
	virtual ~WIIcon() override;
	virtual void Initialize() override;
	void SetClipping(uint32_t xStart,uint32_t yStart,uint32_t width,uint32_t height);
};

#endif