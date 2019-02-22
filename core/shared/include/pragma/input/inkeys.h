#ifndef __INKEYS_H__
#define __INKEYS_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class Action : uint32_t
{
	None = 0,
	MoveForward = 1,
	MoveBackward = MoveForward<<1,
	MoveLeft = MoveBackward<<1,
	MoveRight = MoveLeft<<1,
	Sprint = MoveRight<<1,
	Walk = Sprint<<1,
	Jump = Walk<<1,
	Crouch = Jump<<1,
	Attack = Crouch<<1,
	Attack2 = Attack<<1,
	Attack3 = Attack2<<1,
	Attack4 = Attack3<<1,
	Reload = Attack4<<1,
	Use = Reload<<1
};

REGISTER_BASIC_BITWISE_OPERATORS(Action);

struct DLLNETWORK InputAction
{
	Action action;
	unsigned char task;
};

#endif