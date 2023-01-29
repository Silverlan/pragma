/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __OCCLUSION_CULLING_HANDLER_HPP__
#define __OCCLUSION_CULLING_HANDLER_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/baseentity_handle.h>
#include <pragma/util/util_bsp_tree.hpp>
#include <mathutil/plane.hpp>

class CBaseEntity;
class CModelMesh;
namespace pragma {
	class CSceneComponent;
	class CRasterizationRendererComponent;
	class CParticleSystemComponent;
	class CLightComponent;
	struct DLLCLIENT OcclusionMeshInfo {
		OcclusionMeshInfo(CBaseEntity &ent, CModelMesh &mesh);
		CModelMesh *mesh;
		EntityHandle hEntity;
	};
	class DLLCLIENT OcclusionCullingHandler : public std::enable_shared_from_this<OcclusionCullingHandler> {
	  public:
		virtual ~OcclusionCullingHandler() = default;
		virtual void Update(const Vector3 &camPos) {}
		virtual void Initialize() {}
		// This function will not cull anything, culling behavior has to be implemented by derived classes
		virtual void PerformCulling(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<OcclusionMeshInfo> &culledMeshesOut, bool cullByViewFrustum = true) = 0;
		void PerformCulling(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, std::vector<OcclusionMeshInfo> &culledMeshesOut);
		void PerformCulling(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, std::vector<pragma::CParticleSystemComponent *> &particlesOut);

		virtual void PerformCulling(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<pragma::CParticleSystemComponent *> &particlesOut);
		virtual void PerformCulling(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const std::vector<pragma::CLightComponent *> &lightsIn, std::vector<pragma::CLightComponent *> &lightsOut);
		virtual void PerformCulling(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &origin, float radius, std::vector<OcclusionMeshInfo> &culledMeshesOut);
	  protected:
		OcclusionCullingHandler() = default;
		virtual bool ShouldExamine(CModelMesh &mesh, const Vector3 &pos, bool bViewModel, std::size_t numMeshes, const std::vector<umath::Plane> *optPlanes = nullptr) const;
		virtual bool ShouldExamine(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CBaseEntity &cent, bool &outViewModel, std::vector<umath::Plane> **outPlanes) const;
	};
};

#endif
