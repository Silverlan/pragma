/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WIMAINMENU_LOADGAME_H__
#define __WIMAINMENU_LOADGAME_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"

class DLLCLIENT WIMainMenuLoadGame
	: public WIMainMenuBase
{
public:
	WIMainMenuLoadGame();
	virtual ~WIMainMenuLoadGame() override;
	virtual void Initialize() override;
};


#endif