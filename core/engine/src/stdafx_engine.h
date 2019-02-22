#ifdef _WIN32
	#pragma message("Compiling precompiled headers.\n")
	#define _WIN32_WINNT 0x0501
#endif
#include <pragma/util/boostutil.h>
#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include "pragma/physics/physapi.h"
#include "pragma/engine.h"
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game.h>
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>