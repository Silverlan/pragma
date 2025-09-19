// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <functional>

export module pragma.client.debug.render_filter;

export struct DLLCLIENT DebugRenderFilter {
	std::function<bool(pragma::ShaderGameWorld &)> shaderFilter = nullptr;
	std::function<bool(CMaterial &)> materialFilter = nullptr;
	std::function<bool(CBaseEntity &, CMaterial &)> entityFilter = nullptr;
	std::function<bool(CBaseEntity &, CMaterial *, CModelSubMesh &, pragma::RenderMeshIndex)> meshFilter = nullptr;
};
