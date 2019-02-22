#include "stdafx_client.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/model/c_modelmesh.h"

using namespace pragma;

void OcclusionCullingHandlerBruteForce::PerformCulling(const Scene &scene,std::vector<OcclusionMeshInfo> &culledMeshesOut)
{
	auto &cam = scene.camera;
	auto &posCam = cam->GetPos();
	//auto d = uvec::distance(m_lastLodCamPos,posCam);
	//auto bUpdateLod = (d >= LOD_SWAP_DISTANCE) ? true : false;
	culledMeshesOut.clear();

	auto &ents = scene.GetEntities();
	for(auto &hEnt : ents)
	{
		if(hEnt.IsValid() == false)
			continue;
		auto *ent = static_cast<CBaseEntity*>(hEnt.get());
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent.expired())
			continue;
		bool bViewModel = false;
		std::vector<Plane> *planes = nullptr;
		if((ShouldExamine(scene,*ent,bViewModel,&planes) == true))
		{
			//if(bUpdateLod == true) // Needs to be updated every frame (in case the entity is moving towards or away from us)
			pRenderComponent->GetModelComponent()->UpdateLOD(posCam);
			if(pRenderComponent.valid())
			{
				auto pTrComponent = ent->GetTransformComponent();
				auto &meshes = pRenderComponent->GetLODMeshes();
				auto numMeshes = meshes.size();
				auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
				for(auto itMesh=meshes.begin();itMesh!=meshes.end();++itMesh)
				{
					auto *mesh = static_cast<CModelMesh*>(itMesh->get());
					if(ShouldExamine(*mesh,pos,bViewModel,numMeshes,*planes) == true)
					{
						if(culledMeshesOut.capacity() -culledMeshesOut.size() == 0)
							culledMeshesOut.reserve(culledMeshesOut.capacity() +100);
						culledMeshesOut.push_back(OcclusionMeshInfo{*ent,*mesh});
					}
				}
			}
		}
	}
	//if(bUpdateLod == true)
	//	m_lastLodCamPos = posCam;
}
