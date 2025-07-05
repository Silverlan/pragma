// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
