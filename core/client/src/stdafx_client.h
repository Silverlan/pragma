/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifdef _WIN32
// #pragma message("Compiling precompiled headers.\n")
#define _WIN32_WINNT 0x0501
#define NOMINMAX
#include <Windows.h>
#endif
#include <mathutil/glmutil.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include "pragma/c_engine.h"
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include "pragma/clientstate/clientstate.h"
#include <pragma/game/game.h>
#include "pragma/game/c_game.h"
#include <pragma/entities/baseentity.h>
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/components/base_entity_component.hpp>
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
#include "pragma/lua/converters/gui_element_converter.hpp"
#include "pragma/lua/converters/shader_converter.hpp"
#include "pragma/lua/converters/cast_conversions.hpp"
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#ifdef __linux__
#include <wgui/wihandle.h>
#include "pragma/lua/policies/gui_element_policy.hpp"

#include "pragma/lua/converters/cast_converter.hpp"
#include "pragma/lua/converters/cast_converter_t.hpp"
#include "pragma/lua/converters/gui_element_converter.hpp"
#include "pragma/lua/converters/gui_element_converter_t.hpp"
#include "pragma/lua/converters/shader_converter.hpp"
#include "pragma/lua/converters/shader_converter_t.hpp"
#endif
