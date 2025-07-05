// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
