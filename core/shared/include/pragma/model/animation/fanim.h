#ifndef __FANIM_H__
#define __FANIM_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class DLLNETWORK FAnim : uint32_t
{
	None = 0,
	Loop = 1,
	NoRepeat = 2,
	MoveX = 32,
	MoveZ = 64,
	Autoplay = 128,
	Gesture = 256,
	NoMoveBlend = 512
};
REGISTER_BASIC_BITWISE_OPERATORS(FAnim)

#endif