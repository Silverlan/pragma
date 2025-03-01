/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifdef _WIN32
	// #pragma message("Compiling precompiled headers.\n")
	#define _WIN32_WINNT 0x0501
	#define NOMINMAX
	#include <Windows.h>
#endif

#include <deque>
#include <cmath>
#include <climits>
#include <cfloat>
#include <cassert>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <memory.h>
#include <stdarg.h>
#include <stddef.h>
#include <pragma/lua/luaapi.h>
#include <fstream>
#include <cinttypes>
#include <queue>
#include <any>

#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#ifdef _WIN32
#include <pragma/engine.h>
#include <pragma/networkstate/networkstate.h>
#endif
#include <pragma/serverstate/serverstate.h>
#ifdef _WIN32
#include <pragma/game/game.h>
#include "pragma/game/s_game.h"
#endif
#include <pragma/entities/baseentity.h>
#include "pragma/entities/s_baseentity.h"
#include <pragma/physics/physobj.h>
#include <pragma/lua/luaapi.h>
#include <pragma/audio/alsound.h>
#include <pragma/console/convars.h>
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <mathutil/umath.h>
#include <pragma/entities/entity_component_manager.hpp>
#include <pragma/entities/entity_component_system.hpp>
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>