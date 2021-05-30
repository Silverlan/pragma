/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PARENTMODE_H__
#define __PARENTMODE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class FAttachmentMode : uint8_t
{
	None = 0u,
	PositionOnly = 1u,
	BoneMerge = PositionOnly<<1u,
	UpdateEachFrame = BoneMerge<<1u,
	PlayerView = UpdateEachFrame<<1u,
	PlayerViewYaw = PlayerView<<1u,
	SnapToOrigin = PlayerViewYaw<<1u,
	
	ForceTranslationInPlace = SnapToOrigin<<1u,
	ForceRotationInPlace = ForceTranslationInPlace<<1u,
	ForceInPlace = ForceTranslationInPlace | ForceRotationInPlace
};
REGISTER_BASIC_BITWISE_OPERATORS(FAttachmentMode);

#endif
