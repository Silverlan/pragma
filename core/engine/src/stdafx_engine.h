#ifdef _WIN32
	#pragma message("Compiling precompiled headers.\n")
	#define _WIN32_WINNT 0x0501
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
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <fstream>
#include <cinttypes>
#include <queue>
#include <cstring>
#include <stack>

#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include "pragma/lua/luaapi.h"
#include "pragma/engine.h"
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game.h>
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
