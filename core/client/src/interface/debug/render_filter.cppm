// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:debug.render_filter;

export import :entities.base_entity;
export import :model.mesh;
export import :model.render_mesh_group;
export import :rendering.shaders.scene;

export namespace pragma::debug {
	struct DLLCLIENT DebugRenderFilter {
		std::function<bool(pragma::ShaderGameWorld &)> shaderFilter = nullptr;
		std::function<bool(msys::CMaterial &)> materialFilter = nullptr;
		std::function<bool(pragma::ecs::CBaseEntity &, msys::CMaterial &)> entityFilter = nullptr;
		std::function<bool(pragma::ecs::CBaseEntity &, msys::CMaterial *, pragma::geometry::CModelSubMesh &, pragma::rendering::RenderMeshIndex)> meshFilter = nullptr;
	};
}
