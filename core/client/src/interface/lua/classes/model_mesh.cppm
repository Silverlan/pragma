// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.model_mesh;

export import pragma.shared;

export namespace Lua {
	namespace ModelMesh {
		namespace Client {
			DLLCLIENT void Create(lua::State *l);
		};
	};
	namespace ModelSubMesh {
		namespace Client {
			DLLCLIENT void Create(lua::State *l);
			DLLCLIENT void GetVkMesh(lua::State *l, pragma::geometry::ModelSubMesh &mesh);
			DLLCLIENT void GetTangents(lua::State *l, pragma::geometry::ModelSubMesh &mesh);
			DLLCLIENT void GetBiTangents(lua::State *l, pragma::geometry::ModelSubMesh &mesh);
			DLLCLIENT void GetVertexBuffer(lua::State *l, pragma::geometry::ModelSubMesh &mesh);
			DLLCLIENT void GetVertexWeightBuffer(lua::State *l, pragma::geometry::ModelSubMesh &mesh);
			DLLCLIENT void GetAlphaBuffer(lua::State *l, pragma::geometry::ModelSubMesh &mesh);
			DLLCLIENT void GetIndexBuffer(lua::State *l, pragma::geometry::ModelSubMesh &mesh);
		};
	};
};
