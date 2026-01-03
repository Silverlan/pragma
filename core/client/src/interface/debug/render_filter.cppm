// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:debug.render_filter;

export import :entities.base_entity;
export import :model.mesh;
export import :model.render_mesh_group;
export import :rendering.shaders.scene;

export namespace pragma::debug {
	struct DLLCLIENT DebugRenderFilter {
		std::function<bool(ShaderGameWorld &)> shaderFilter = nullptr;
		std::function<bool(material::CMaterial &)> materialFilter = nullptr;
		std::function<bool(ecs::CBaseEntity &, material::CMaterial &)> entityFilter = nullptr;
		std::function<bool(ecs::CBaseEntity &, material::CMaterial *, geometry::CModelSubMesh &, rendering::RenderMeshIndex)> meshFilter = nullptr;
	};
}
