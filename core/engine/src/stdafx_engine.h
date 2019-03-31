#ifdef _WIN32
	#pragma message("Compiling precompiled headers.\n")
	#define _WIN32_WINNT 0x0501
#endif

#include <deque>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
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
#include <luabind/boost_tuple_extended.hpp>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <fstream>
#include <cinttypes>
#include <queue>
#include <cstring>
#include <stack>

#include <pragma/util/boostutil.h>
#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include "pragma/physics/physapi.h"
#include "pragma/engine.h"
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game.h>
#include <luasystem.h>
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
