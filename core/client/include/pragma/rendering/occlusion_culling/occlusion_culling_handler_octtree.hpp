#ifndef __OCCLUSION_CULLING_HANDLER_BRUTE_OCTTREE_HPP__
#define __OCCLUSION_CULLING_HANDLER_BRUTE_OCTTREE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"

namespace pragma
{
	class DLLCLIENT OcclusionCullingHandlerOctTree
		: public OcclusionCullingHandler
	{
	public:
		OcclusionCullingHandlerOctTree()=default;
		virtual void PerformCulling(const Scene &scene,std::vector<OcclusionMeshInfo> &culledMeshesOut) override;
	private:
		Vector3 m_lastLodCamPos {};
	};
};

#endif
