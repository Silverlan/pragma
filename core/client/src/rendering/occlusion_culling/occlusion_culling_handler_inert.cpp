#include "stdafx_client.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_inert.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_iterator.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

void OcclusionCullingHandlerInert::PerformCulling(
	const pragma::rendering::RasterizationRenderer &renderer,const Vector3 &camPos,
	std::vector<pragma::CParticleSystemComponent*> &particlesOut
)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	particlesOut.reserve(entIt.GetCount());
	particlesOut.clear();
	for(auto *ent : entIt)
		particlesOut.push_back(ent->GetComponent<pragma::CParticleSystemComponent>().get());
}
void OcclusionCullingHandlerInert::PerformCulling(
	const pragma::rendering::RasterizationRenderer &renderer,const Vector3 &camPos,
	std::vector<OcclusionMeshInfo> &culledMeshesOut,bool cullByViewFrustum
)
{
	auto &scene = renderer.GetScene();
	//auto d = uvec::distance(m_lastLodCamPos,posCam);
	//auto bUpdateLod = (d >= LOD_SWAP_DISTANCE) ? true : false;
	culledMeshesOut.clear();

	std::vector<CBaseEntity*> *ents;
	c_game->GetEntities(&ents);
	for(auto *ent : *ents)
	{
		if(ent == nullptr)
			continue;
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent.expired())
			continue;
		bool bViewModel = false;
		if((ent->IsSpawned() == true && pRenderComponent->GetModelComponent().valid() && pRenderComponent->GetModelComponent()->GetModel() != nullptr && pRenderComponent->ShouldDraw(camPos) != false))
		{
			//if(bUpdateLod == true) // Needs to be updated every frame (in case the entity is moving towards or away from us)
			pRenderComponent->GetModelComponent()->UpdateLOD(camPos);
			if(pRenderComponent.valid())
			{
				auto pTrComponent = ent->GetTransformComponent();
				auto &meshes = pRenderComponent->GetLODMeshes();
				auto numMeshes = meshes.size();
				auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
				for(auto itMesh=meshes.begin();itMesh!=meshes.end();++itMesh)
				{
					auto *mesh = static_cast<CModelMesh*>(itMesh->get());
					if(culledMeshesOut.capacity() -culledMeshesOut.size() == 0)
						culledMeshesOut.reserve(culledMeshesOut.capacity() +100);
					culledMeshesOut.push_back(OcclusionMeshInfo{*ent,*mesh});
				}
			}
		}
	}
	//if(bUpdateLod == true)
	//	m_lastLodCamPos = posCam;
}
