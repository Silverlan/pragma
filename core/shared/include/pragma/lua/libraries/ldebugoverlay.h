/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDEBUGOVERLAY_H__
#define __LDEBUGOVERLAY_H__
#include <pragma/definitions.h>
#include <pragma/engine.h>
#include "pragma/lua/ldefinitions.h"
DLLNETWORK int Lua_debugoverlay_DrawLine(lua_State *l);
DLLNETWORK int Lua_debugoverlay_DrawBox(lua_State *l);
#endif
