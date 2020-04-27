/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LSHADERINFO_H__
#define __LSHADERINFO_H__
#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include <sharedutils/util_shaderinfo.hpp>
DLLNETWORK void Lua_ShaderInfo_GetName(lua_State *l,util::ShaderInfo *shader);
#endif