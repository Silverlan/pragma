/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LDEBUGOVERLAY_H__
#define __C_LDEBUGOVERLAY_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/debug/c_debugoverlay.h"

namespace Lua
{
	namespace DebugRenderer
	{
		namespace Client
		{
			DLLCLIENT int DrawPoints(lua_State *l);
			DLLCLIENT int DrawLines(lua_State *l);
			DLLCLIENT int DrawPoint(lua_State *l);
			DLLCLIENT int DrawLine(lua_State *l);
			DLLCLIENT int DrawBox(lua_State *l);
			DLLCLIENT int DrawMeshes(lua_State *l);
			DLLCLIENT int DrawSphere(lua_State *l);
			DLLCLIENT int DrawTruncatedCone(lua_State *l);
			DLLCLIENT int DrawCylinder(lua_State *l);
			DLLCLIENT int DrawCone(lua_State *l);
			DLLCLIENT int DrawAxis(lua_State *l);
			DLLCLIENT int DrawText(lua_State *l);
			DLLCLIENT int DrawPath(lua_State *l);
			DLLCLIENT int DrawSpline(lua_State *l);
			DLLCLIENT int DrawPlane(lua_State *l);
			DLLCLIENT int DrawFrustum(lua_State *l);
			namespace Object
			{
				DLLCLIENT void Remove(lua_State *l,::DebugRenderer::BaseObject &o);
				DLLCLIENT void IsValid(lua_State *l,::DebugRenderer::BaseObject &o);
				DLLCLIENT void SetPos(lua_State *l,::DebugRenderer::BaseObject &o,const Vector3 &pos);
				DLLCLIENT void GetPos(lua_State *l,::DebugRenderer::BaseObject &o);
				DLLCLIENT void SetRotation(lua_State *l,::DebugRenderer::BaseObject &o,const Quat &rot);
				DLLCLIENT void GetRotation(lua_State *l,::DebugRenderer::BaseObject &o);
				DLLCLIENT void SetAngles(lua_State *l,::DebugRenderer::BaseObject &o,const EulerAngles &ang);
				DLLCLIENT void GetAngles(lua_State *l,::DebugRenderer::BaseObject &o);
				DLLCLIENT void IsVisible(lua_State *l,::DebugRenderer::BaseObject &o);
				DLLCLIENT void SetVisible(lua_State *l,::DebugRenderer::BaseObject &o,bool b);
			};
		};
	};
};

#endif