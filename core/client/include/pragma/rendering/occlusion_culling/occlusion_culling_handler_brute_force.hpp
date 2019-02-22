#ifndef __OCCLUSION_CULLING_HANDLER_BRUTE_FORCE_HPP__
#define __OCCLUSION_CULLING_HANDLER_BRUTE_FORCE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"

namespace pragma
{
	class DLLCLIENT OcclusionCullingHandlerBruteForce
		: public OcclusionCullingHandler
	{
	public:
		OcclusionCullingHandlerBruteForce()=default;
		virtual void PerformCulling(const Scene &scene,std::vector<pragma::OcclusionMeshInfo> &culledMeshesOut) override;
	private:
	};
};

#endif
