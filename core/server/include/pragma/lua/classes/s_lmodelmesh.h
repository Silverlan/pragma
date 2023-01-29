/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LMODELMESH_H__
#define __S_LMODELMESH_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class ModelMesh;

namespace Lua {
	namespace ModelMesh {
		namespace Server {
			DLLSERVER void Create(lua_State *l);
		};
	};
	namespace ModelSubMesh {
		namespace Server {
			DLLSERVER void Create(lua_State *l);
			DLLSERVER void CreateQuad(lua_State *l, float size);
			DLLSERVER void CreateBox(lua_State *l, const Vector3 &min, const Vector3 &max);
			DLLSERVER void CreateSphere(lua_State *l, const Vector3 &origin, float radius, uint32_t recursionLevel);
			DLLSERVER void CreateSphere(lua_State *l, const Vector3 &origin, float radius);
			DLLSERVER void CreateCylinder(lua_State *l, float startRadius, float length, uint32_t segmentCount);
			DLLSERVER void CreateCone(lua_State *l, float startRadius, float length, float endRadius, uint32_t segmentCount);
			DLLSERVER void CreateCircle(lua_State *l, float radius, bool doubleSided, uint32_t segmentCount);
			DLLSERVER void CreateRing(lua_State *l, float innerRadius, float outerRadius, bool doubleSided, uint32_t segmentCount);
		};
	};
};

#endif
