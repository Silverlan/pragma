/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LMODELMESH_H__
#define __C_LMODELMESH_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"

class ModelMesh;
class ModelSubMesh;

namespace Lua {
	namespace ModelMesh {
		namespace Client {
			DLLCLIENT void Create(lua_State *l);
		};
	};
	namespace ModelSubMesh {
		namespace Client {
			DLLCLIENT void Create(lua_State *l);
			DLLCLIENT void GetVkMesh(lua_State *l, ::ModelSubMesh &mesh);
			DLLCLIENT void GetTangents(lua_State *l, ::ModelSubMesh &mesh);
			DLLCLIENT void GetBiTangents(lua_State *l, ::ModelSubMesh &mesh);
			DLLCLIENT void GetVertexBuffer(lua_State *l, ::ModelSubMesh &mesh);
			DLLCLIENT void GetVertexWeightBuffer(lua_State *l, ::ModelSubMesh &mesh);
			DLLCLIENT void GetAlphaBuffer(lua_State *l, ::ModelSubMesh &mesh);
			DLLCLIENT void GetIndexBuffer(lua_State *l, ::ModelSubMesh &mesh);
		};
	};
};

#endif
