// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __OCCLUSION_CULLING_HANDLER_BRUTE_OCTTREE_HPP__
#define __OCCLUSION_CULLING_HANDLER_BRUTE_OCTTREE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"

namespace pragma {
	class DLLCLIENT OcclusionCullingHandlerOctTree : public OcclusionCullingHandler {
	  public:
		OcclusionCullingHandlerOctTree() = default;
		virtual void PerformCulling(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<pragma::OcclusionMeshInfo> &culledMeshesOut, bool cullByViewFrustum = true) override;
	  private:
		Vector3 m_lastLodCamPos {};
	};
};

#endif
