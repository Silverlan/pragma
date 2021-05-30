/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CULLED_MESH_DATA_HPP__
#define __CULLED_MESH_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/rendersystem.h"
#include <vector>
#include <memory>
#include <unordered_map>

class BaseEntity;
struct ShaderMeshContainer;
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
