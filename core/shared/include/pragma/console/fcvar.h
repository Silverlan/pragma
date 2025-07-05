// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __FCVAR_H__
#define __FCVAR_H__

#include "pragma/definitions.h"
#include <mathutil/umath.h>

enum class DLLNETWORK ConVarFlags : uint32_t {
	None = 0,
	Cheat = 1,
	Singleplayer = Cheat << 1,
	Userinfo = Singleplayer << 1,
	Replicated = Userinfo << 1,
	Archive = Replicated << 1,
	Notify = Archive << 1,
	JoystickAxisContinuous = Notify << 1,
	JoystickAxisSingle = JoystickAxisContinuous << 1,
	Hidden = JoystickAxisSingle << 1,
	Password = Hidden << 1u,

	Last = Hidden
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(ConVarFlags);

#endif
