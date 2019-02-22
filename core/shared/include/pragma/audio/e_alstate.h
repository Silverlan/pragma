#ifndef __E_ALSTATE_H__
#define __E_ALSTATE_H__

#include "pragma/networkdefinitions.h"

#ifndef OPENAL
	enum class DLLNETWORK ALState : uint32_t
	{
		NoError = 0,
		Initial = 0x1011,
		Playing = 0x1012,
		Paused = 0x1013,
		Stopped = 0x1014
	};
#endif

#endif
