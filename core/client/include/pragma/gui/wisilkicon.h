/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WISILKICON_H__
#define __WISILKICON_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/wiicon.h"

class DLLCLIENT WISilkIcon
	: public WIIcon
{
public:
	WISilkIcon();
	virtual ~WISilkIcon() override;
	virtual void Initialize() override;
	void SetIcon(std::string icon);
};

#endif