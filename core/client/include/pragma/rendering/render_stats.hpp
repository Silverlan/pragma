#ifndef __PRAGMA_RENDER_STATS_HPP__
#define __PRAGMA_RENDER_STATS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <material.h>
#include <sharedutils/util_weak_handle.hpp>
#include <vector>

namespace prosper {class Shader;};

class CModelSubMesh;
struct DLLCLIENT RenderPassStats
{
	std::vector<EntityHandle> entities;
	std::vector<MaterialHandle> materials;
	std::vector<util::WeakHandle<prosper::Shader>> shaders;
	std::vector<std::shared_ptr<const CModelSubMesh>> meshes;
};

struct DLLCLIENT RenderStats
{
	RenderPassStats lightingPass;
	RenderPassStats prepass;
	RenderPassStats transparencyPass;
};

#endif
