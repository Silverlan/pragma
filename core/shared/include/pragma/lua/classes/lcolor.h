// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LCOLOR_H__
#define __LCOLOR_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <mathutil/color.h>

namespace Lua {
	namespace Color {
		DLLNETWORK void Copy(lua_State *l, ::Color &col);
		DLLNETWORK void Set(lua_State *l, ::Color &col, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
		DLLNETWORK void Lerp(lua_State *l, ::Color &col, ::Color &other, float factor);
		DLLNETWORK void ToVector4(lua_State *l, ::Color &col);
		DLLNETWORK void ToVector(lua_State *l, ::Color &col);
	};
};

#endif
