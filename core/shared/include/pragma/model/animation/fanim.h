/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __FANIM_H__
#define __FANIM_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class DLLNETWORK FAnim : uint32_t {
	None = 0,
	Loop = 1,
	NoRepeat = 2,
	MoveX = 32,
	MoveZ = 64,
	Autoplay = 128,
	Gesture = 256,
	NoMoveBlend = 512,

	Count = 7
};
REGISTER_BASIC_BITWISE_OPERATORS(FAnim)

#endif
