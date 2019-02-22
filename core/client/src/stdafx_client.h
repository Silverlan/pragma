#ifdef _WIN32
	#pragma message("Compiling precompiled headers.\n")
#endif
#include <pragma/util/boostutil.h>
#ifdef _WIN32
	#define _WIN32_WINNT 0x0501
	#include <Windows.h>
#endif
#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include <pragma/physics/physapi.h>
#include "pragma/c_engine.h"
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include "pragma/clientstate/clientstate.h"
#include <pragma/game/game.h>
#include "pragma/game/c_game.h"
#include <pragma/entities/baseentity.h>
#include "pragma/entities/c_baseentity.h"
#include <pragma/physics/physobj.h>
#include <pragma/lua/luaapi.h>
#include <pragma/audio/alsound.h>
#include <pragma/console/convars.h>
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <mathutil/umath.h>
#include <pragma/entities/entity_component_manager.hpp>
#include "pragma/entities/entity_component_system.hpp"
#include <pragma/lua/l_entity_handles.hpp>
#include "pragma/lua/c_lentity_handles.hpp"
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>