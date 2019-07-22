#ifndef __FCVAR_H__
#define __FCVAR_H__

#include "pragma/definitions.h"
#include <mathutil/umath.h>

enum class DLLENGINE ConVarFlags : uint32_t
{
	None = 0,
	Cheat = 1,
	Singleplayer = Cheat<<1,
	Userinfo = Singleplayer<<1,
	Replicated = Userinfo<<1,
	Archive = Replicated<<1,
	Notify = Archive<<1,
	JoystickAxisContinuous = Notify<<1,
	JoystickAxisSingle = JoystickAxisContinuous<<1,
	Hidden = JoystickAxisSingle<<1,

	Last = Hidden
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(ConVarFlags);

#endif