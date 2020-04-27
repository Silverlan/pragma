/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __AI_TASK_H__
#define __AI_TASK_H__

#include "pragma/serverdefinitions.h"

// These also have to be registered in SGame constructor!
namespace pragma
{
	namespace ai
	{
		enum class DLLSERVER Task : uint32_t
		{
			MoveToTarget = 0,
			PlayAnimation,
			PlayActivity,
			PlayLayeredAnimation,
			PlayLayeredActivity,
			MoveRandom,
			PlaySound,
			DebugPrint,
			DebugDrawText,
			Decorator,
			Wait,
			TurnToTarget,
			Random,
			LookAtTarget,
			Event
		};
	};
};

#endif
