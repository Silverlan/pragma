/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_LDEBUGOVERLAY_H__
#define __S_LDEBUGOVERLAY_H__
#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace DebugRenderer
	{
		namespace Server
		{
			DLLSERVER int DrawPoint(lua_State *l);
			DLLSERVER int DrawLine(lua_State *l);
			DLLSERVER int DrawBox(lua_State *l);
			DLLSERVER int DrawSphere(lua_State *l);
			DLLSERVER int DrawCone(lua_State *l);
			DLLSERVER int DrawTruncatedCone(lua_State *l);
			DLLSERVER int DrawCylinder(lua_State *l);
			DLLSERVER int DrawAxis(lua_State *l);
			DLLSERVER int DrawText(lua_State *l);
			DLLSERVER int DrawPath(lua_State *l);
			DLLSERVER int DrawSpline(lua_State *l);
			DLLSERVER int DrawPlane(lua_State *l);
		};
	};
};

#endif