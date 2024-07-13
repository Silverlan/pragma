/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __OBSERVERMODE_H__
#define __OBSERVERMODE_H__

#include "pragma/networkdefinitions.h"
enum class ObserverMode : int {
	None,
	FirstPerson,
	ThirdPerson,
	Shoulder,
	Roaming,
	Count,
};

#endif
