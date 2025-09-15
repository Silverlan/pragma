// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_chc.hpp"
#include "pragma/entities/baseworld.h"

import pragma.client.entities.components.world;

using namespace pragma;

extern CGame *c_game;

void OcclusionCullingHandlerCHC::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<OcclusionMeshInfo> &culledMeshesOut, bool cullByViewFrustum)
{
	auto *entWorld = static_cast<pragma::CWorldComponent *>(c_game->GetWorld()); // TODO
	if(entWorld == nullptr)
		return;
	auto chcController = entWorld->GetCHCController();
	if(chcController == nullptr)
		return;
	culledMeshesOut = chcController->PerformCulling();
}
