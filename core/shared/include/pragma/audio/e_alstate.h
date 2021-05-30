/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
