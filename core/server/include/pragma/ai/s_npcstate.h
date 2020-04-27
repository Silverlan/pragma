/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_NPCSTATE_H__
#define __S_NPCSTATE_H__

#include "pragma/serverdefinitions.h"

enum class NPCSTATE : int
{
	NONE,
	IDLE,
	ALERT,
	COMBAT,
	SCRIPT
};

#endif