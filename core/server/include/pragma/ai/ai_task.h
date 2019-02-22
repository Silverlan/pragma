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
