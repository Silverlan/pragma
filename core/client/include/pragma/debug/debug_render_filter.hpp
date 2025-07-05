// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __DEBUG_RENDER_FILTER_HPP__
#define __DEBUG_RENDER_FILTER_HPP__

#include "pragma/clientdefinitions.h"
#include <functional>

namespace pragma {
	class ShaderGameWorld;
	using RenderMeshIndex = uint32_t;
};

class CMaterial;
class CBaseEntity;
class CModelSubMesh;
struct DLLCLIENT DebugRenderFilter {
	std::function<bool(pragma::ShaderGameWorld &)> shaderFilter = nullptr;
	std::function<bool(CMaterial &)> materialFilter = nullptr;
	std::function<bool(CBaseEntity &, CMaterial &)> entityFilter = nullptr;
	std::function<bool(CBaseEntity &, CMaterial *, CModelSubMesh &, pragma::RenderMeshIndex)> meshFilter = nullptr;
};

#endif
