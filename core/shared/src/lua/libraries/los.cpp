// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/los.h"
#include <pragma/lua/luaapi.h>

int64_t Lua::os::time_since_epoch(lua_State *l) { return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count(); }
