// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_iterator.hpp>

import pragma.client.entities.components.render;

using namespace pragma;

extern CGame *c_game;

void OcclusionCullingHandlerBruteForce::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<OcclusionMeshInfo> &culledMeshesOut, bool cullByViewFrustum)
{
	//auto d = uvec::distance(m_lastLodCamPos,posCam);
	//auto bUpdateLod = (d >= LOD_SWAP_DISTANCE) ? true : false;
	culledMeshesOut.clear();

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	for(auto *e : entIt) {
		if(e == nullptr)
			continue;
		auto *ent = static_cast<CBaseEntity *>(e);
		if(ent->IsInScene(scene) == false)
			continue;
		auto pRenderComponent = ent->GetRenderComponent();
		bool bViewModel = false;
		std::vector<umath::Plane> *planes = nullptr;
		if((ShouldExamine(scene, renderer, *ent, bViewModel, cullByViewFrustum ? &planes : nullptr) == true)) {
			//if(bUpdateLod == true) // Needs to be updated every frame (in case the entity is moving towards or away from us)
			//pRenderComponent->GetModelComponent()->UpdateLOD(camPos);
			if(pRenderComponent) {
				auto pTrComponent = ent->GetTransformComponent();
				auto &meshes = pRenderComponent->GetLODMeshes();
				auto numMeshes = meshes.size();
				auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
				for(auto itMesh = meshes.begin(); itMesh != meshes.end(); ++itMesh) {
					auto *mesh = static_cast<CModelMesh *>(itMesh->get());
					if(ShouldExamine(*mesh, pos, bViewModel, numMeshes, planes) == true) {
						if(culledMeshesOut.capacity() - culledMeshesOut.size() == 0)
							culledMeshesOut.reserve(culledMeshesOut.capacity() + 100);
						culledMeshesOut.push_back(OcclusionMeshInfo {*ent, *mesh});
					}
				}
			}
		}
	}
	//if(bUpdateLod == true)
	//	m_lastLodCamPos = posCam;
}
