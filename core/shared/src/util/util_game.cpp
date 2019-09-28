#include "pragma/util/util_game.hpp"
#include "pragma/networkstate/networkstate.h"
#include <sharedutils/util_library.hpp>

#pragma optimize("",off)
util::ParallelJob<std::vector<Vector2>&> util::generate_lightmap_uvs(NetworkState &nwState,uint32_t atlastWidth,uint32_t atlasHeight,const std::vector<Vertex> &verts,const std::vector<uint32_t> &tris)
{
	auto lib = nwState.InitializeLibrary("pr_uvatlas");
	if(lib == nullptr)
		return {};
	auto *fGenerateAtlasUvs = lib->FindSymbolAddress<void(*)(uint32_t,uint32_t,const std::vector<Vertex>&,const std::vector<uint32_t>&,util::ParallelJob<std::vector<Vector2>&>&)>("pr_uvatlas_generate_atlas_uvs");
	if(fGenerateAtlasUvs == nullptr)
		return {};
	util::ParallelJob<std::vector<Vector2>&> job {};
	fGenerateAtlasUvs(atlastWidth,atlasHeight,verts,tris,job);
	if(job.IsValid() == false)
		return {};
	return job;
}
#pragma optimize("",on)
