/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_inert.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_iterator.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

void OcclusionCullingHandlerInert::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<pragma::CParticleSystemComponent *> &particlesOut)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	particlesOut.reserve(entIt.GetCount());
	particlesOut.clear();
	for(auto *ent : entIt)
		particlesOut.push_back(ent->GetComponent<pragma::CParticleSystemComponent>().get());
}
void OcclusionCullingHandlerInert::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<OcclusionMeshInfo> &culledMeshesOut, bool cullByViewFrustum)
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
		auto *pRenderComponent = ent->GetRenderComponent();
		bool bViewModel = false;
		if((ent->IsSpawned() == true && pRenderComponent->GetModelComponent() && pRenderComponent->GetModelComponent()->GetModel() != nullptr && pRenderComponent->ShouldDraw() != false)) {
			//if(bUpdateLod == true) // Needs to be updated every frame (in case the entity is moving towards or away from us)
			//pRenderComponent->GetModelComponent()->UpdateLOD(camPos);
			if(pRenderComponent) {
				auto pTrComponent = ent->GetTransformComponent();
				auto &meshes = pRenderComponent->GetLODMeshes();
				auto numMeshes = meshes.size();
				auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
				for(auto itMesh = meshes.begin(); itMesh != meshes.end(); ++itMesh) {
					auto *mesh = static_cast<CModelMesh *>(itMesh->get());
					if(culledMeshesOut.capacity() - culledMeshesOut.size() == 0)
						culledMeshesOut.reserve(culledMeshesOut.capacity() + 100);
					culledMeshesOut.push_back(OcclusionMeshInfo {*ent, *mesh});
				}
			}
		}
	}
	//if(bUpdateLod == true)
	//	m_lastLodCamPos = posCam;
}
