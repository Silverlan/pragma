// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LDEBUGOVERLAY_H__
#define __LDEBUGOVERLAY_H__
#include <pragma/definitions.h>
#include <pragma/engine.h>
#include "pragma/lua/ldefinitions.h"
DLLNETWORK int Lua_debugoverlay_DrawLine(lua_State *l);
DLLNETWORK int Lua_debugoverlay_DrawBox(lua_State *l);
#endif
