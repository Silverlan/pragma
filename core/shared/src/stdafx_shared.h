/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifdef _WIN32
//#pragma message("Compiling precompiled headers.\n")
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
#include <cstring>
#include <assert.h>
#ifdef __linux__
#include <dlfcn.h>
#endif
#include <cstdio>
#include <any>

#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include <pragma/lua/luaapi.h>
#include "pragma/audio/alsound.h"
#include <pragma/console/convars.h>
#include <pragma/console/conout.h>
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <mathutil/umath.h>
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/entity_component_system.hpp"
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#ifdef __linux__
#include "pragma/lua/policies/core_policies.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/policies/file_policy.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/game_policy.hpp"
#include "pragma/lua/policies/generic_policy.hpp"
#include "pragma/lua/policies/handle_policy.hpp"
#include "pragma/lua/policies/network_state_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/policies/pair_policy.hpp"
#include "pragma/lua/policies/property_policy.hpp"
#include "pragma/lua/policies/shared_from_this_policy.hpp"
#include "pragma/lua/policies/string_view_policy.hpp"
#include "pragma/lua/policies/tuple_policy.hpp"
#include "pragma/lua/policies/vector_policy.hpp"
#include <pragma/entities/components/panima_component_channel_submitter_def.hpp>

#include "pragma/lua/converters/alias_converter_t.hpp"
#include "pragma/lua/converters/cast_converter_t.hpp"
#include "pragma/lua/converters/file_converter_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/property_converter_t.hpp"
#include "pragma/lua/converters/string_view_converter_t.hpp"
#include "pragma/lua/converters/thread_pool_converter_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#endif
