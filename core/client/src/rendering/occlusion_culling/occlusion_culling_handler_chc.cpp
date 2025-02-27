/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_chc.hpp"
#include "pragma/rendering/occlusion_culling/chc.hpp"

using namespace pragma;

extern DLLCLIENT CGame *c_game;

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
