/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LSHADERINFO_H__
#define __C_LSHADERINFO_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
namespace util {class ShaderInfo;};

DLLCLIENT void Lua_ShaderInfo_GetID(lua_State *l,util::ShaderInfo &shader);
#endif