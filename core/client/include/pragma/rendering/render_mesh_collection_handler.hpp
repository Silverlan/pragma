/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __RENDER_MESH_COLLECTION_HANDLER_HPP__
#define __RENDER_MESH_COLLECTION_HANDLER_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <vector>
#include <mathutil/uvec.h>
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/c_renderflags.h"

namespace pragma
{
	class CParticleSystemComponent;
	class CSceneComponent;
};
namespace pragma::rendering
{
	struct CulledMeshData;
	class RasterizationRenderer;
	struct DLLCLIENT RenderMeshCollectionHandler
	{
	public:
		enum class ResultFlags : uint8_t
		{
			None = 0u,
			HasGlowMeshes = 1u
		};
		RenderMeshCollectionHandler()=default;
		const std::vector<pragma::OcclusionMeshInfo> &PerformOcclusionCulling(CSceneComponent &scene,const RasterizationRenderer &renderer,const Vector3 &posCam,bool cullByViewFrustum=true);
		ResultFlags GenerateOptimizedRenderObjectStructures(CSceneComponent &scene,const RasterizationRenderer &renderer,const Vector3 &posCam,FRender renderFlags,RenderMode renderMode,bool useGlowMeshes,bool useTranslucentMeshes);
	
		const std::vector<pragma::OcclusionMeshInfo> &GetOcclusionFilteredMeshes() const;
		std::vector<pragma::OcclusionMeshInfo> &GetOcclusionFilteredMeshes();

		const std::vector<pragma::CParticleSystemComponent*> &GetOcclusionFilteredParticleSystems() const;
		std::vector<pragma::CParticleSystemComponent*> &GetOcclusionFilteredParticleSystems();

		const std::unordered_map<RenderMode,std::shared_ptr<CulledMeshData>> &GetRenderMeshData() const;
		std::unordered_map<RenderMode,std::shared_ptr<CulledMeshData>> &GetRenderMeshData();

		std::shared_ptr<CulledMeshData> GetRenderMeshData(RenderMode renderMode) const;
	private:
		std::vector<pragma::OcclusionMeshInfo> m_culledMeshes;
		std::vector<pragma::CParticleSystemComponent*> m_culledParticles;
		std::unordered_map<RenderMode,std::shared_ptr<CulledMeshData>> m_culledMeshData;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::RenderMeshCollectionHandler::ResultFlags)

#endif
