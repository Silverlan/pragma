#include "stdafx_client.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_chc.hpp"
#include "pragma/rendering/occlusion_culling/chc.hpp"

using namespace pragma;

extern DLLCLIENT CGame *c_game;

void OcclusionCullingHandlerCHC::PerformCulling(
	const pragma::rendering::RasterizationRenderer &renderer,const Vector3 &camPos,
	std::vector<OcclusionMeshInfo> &culledMeshesOut,bool cullByViewFrustum
)
{
	auto *entWorld = static_cast<pragma::CWorldComponent*>(c_game->GetWorld()); // TODO
	if(entWorld == nullptr)
		return;
	auto chcController = entWorld->GetCHCController();
	if(chcController == nullptr)
		return;
	culledMeshesOut = chcController->PerformCulling();
}
