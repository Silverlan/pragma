/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __MOVETYPES_H__
#define __MOVETYPES_H__
#include "pragma/networkdefinitions.h"
enum class MOVETYPE : int
{
	NONE,
	WALK,
	FLY,
	NOCLIP,
	FREE,
	PHYSICS
};
#endif