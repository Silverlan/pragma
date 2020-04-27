/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LCOLOR_H__
#define __LCOLOR_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <mathutil/color.h>

namespace Lua
{
	namespace Color
	{
		DLLNETWORK void Copy(lua_State *l,::Color *col);
		DLLNETWORK void Set(lua_State *l,::Color *col,unsigned char r,unsigned char g,unsigned char b,unsigned char a);
		DLLNETWORK void Lerp(lua_State *l,::Color *col,::Color *other,float factor);
		DLLNETWORK void ToVector4(lua_State *l,::Color *col);
		DLLNETWORK void ToVector(lua_State *l,::Color *col);
	};
};

#endif
