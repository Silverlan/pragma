#ifndef __CULLED_MESH_DATA_HPP__
#define __CULLED_MESH_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include <vector>
#include <memory>
#include <unordered_map>

class BaseEntity;
namespace pragma::rendering
{
	struct DLLCLIENT CulledMeshData
	{
		CulledMeshData()=default;
		CulledMeshData(const CulledMeshData&)=delete;
		CulledMeshData &operator=(const CulledMeshData&)=delete;
		std::unordered_map<BaseEntity*,bool> processed = {};
		std::vector<std::unique_ptr<ShaderMeshContainer>> containers = {};
		std::vector<std::unique_ptr<RenderSystem::MaterialMeshContainer>> glowMeshes = {};
		std::vector<std::unique_ptr<RenderSystem::TranslucentMesh>> translucentMeshes = {};
	};
};

#endif
