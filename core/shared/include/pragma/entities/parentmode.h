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
	SnapToOrigin = PlayerViewYaw<<1u
};
REGISTER_BASIC_BITWISE_OPERATORS(FAttachmentMode);

#endif
