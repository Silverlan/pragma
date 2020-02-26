#include "stdafx_client.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_octtree.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/math/intersection.h>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

template<class T>
	void iterate_occlusion_tree(const typename OcclusionOctree<T>::Node &node,std::vector<OcclusionMeshInfo> &culledMeshes,const std::vector<Plane> *optFrustumPlanes,const std::function<void(const T&)> &fObjectCallback)
{
	if(node.IsEmpty() == true)
		return;
	auto &nodeBounds = node.GetWorldBounds();
	if(optFrustumPlanes && Intersection::AABBInPlaneMesh(nodeBounds.first,nodeBounds.second,*optFrustumPlanes) == INTERSECT_OUTSIDE)
		return;
	auto &objs = node.GetObjects();
	for(auto &o : objs)
		fObjectCallback(o);
	if(node.GetChildObjectCount() == 0)
		return;
	auto *children = node.GetChildren();
	if(children == nullptr)
		return;
	for(auto &c : *children)
		iterate_occlusion_tree(static_cast<typename OcclusionOctree<T>::Node&>(*c),culledMeshes,optFrustumPlanes,fObjectCallback);
}

void OcclusionCullingHandlerOctTree::PerformCulling(
	const rendering::RasterizationRenderer &renderer,const Vector3 &camPos,
	std::vector<OcclusionMeshInfo> &culledMeshesOut,bool cullByViewFrustum
)
{
	// TODO: Is this function still being used somewhere? If not, get rid of it!
	auto &scene = renderer.GetScene();
	auto d = uvec::distance(m_lastLodCamPos,camPos);
	m_lastLodCamPos = camPos;
	auto bUpdateLod = (d >= LOD_SWAP_DISTANCE) ? true : false;
	culledMeshesOut.clear();

	// View-models are just added without checking
	for(auto *pRenderComponent : pragma::CRenderComponent::GetViewEntities())
	{
		auto &lodMeshes = pRenderComponent->GetLODMeshes();
		for(auto &mesh : lodMeshes)
			culledMeshesOut.push_back({*static_cast<CBaseEntity*>(&pRenderComponent->GetEntity()),*static_cast<CModelMesh*>(mesh.get())});
	}

	auto &dynOctree = scene.GetOcclusionOctree();
	auto &root = dynOctree.GetRootNode();
	// TODO: Planes
	iterate_occlusion_tree<CBaseEntity*>(root,culledMeshesOut,cullByViewFrustum ? &renderer.GetFrustumPlanes() : nullptr,[this,&renderer,&scene,&bUpdateLod,&camPos,&culledMeshesOut,cullByViewFrustum](const CBaseEntity *cent) {
		auto *ent = const_cast<CBaseEntity*>(cent);
		assert(ent != nullptr);
		if(ent == nullptr)
		{
			// This should NEVER occur, but seems to anyway in some rare cases
			Con::cerr<<"ERROR: NULL Entity in dynamic scene occlusion octree! Ignoring..."<<Con::endl;
			return;
		}
		// World geometry is handled separately
		if(ent->IsWorld() == true)
			return;
		bool bViewModel = false;
		std::vector<Plane> *planes = nullptr;
		if(ShouldExamine(renderer,*ent,bViewModel,cullByViewFrustum ? &planes : nullptr) == false)
			return;
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent.expired())
			return;
		auto pTrComponent = ent->GetTransformComponent();
		if(bUpdateLod == true)
		{
			auto &mdlComponent = pRenderComponent->GetModelComponent();
			if(mdlComponent.valid())
				static_cast<pragma::CModelComponent&>(*mdlComponent).UpdateLOD(camPos);
		}
		auto exemptFromCulling = pRenderComponent->IsExemptFromOcclusionCulling();
		auto &meshes = pRenderComponent->GetLODMeshes();
		auto numMeshes = meshes.size();
		auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
		for(auto &mesh : meshes)
		{
			auto *cmesh = static_cast<CModelMesh*>(mesh.get());
			if(cullByViewFrustum == true && exemptFromCulling == false && ShouldExamine(*cmesh,pos,bViewModel,numMeshes,planes) == false)
				continue;
			if(culledMeshesOut.capacity() -culledMeshesOut.size() == 0)
				culledMeshesOut.reserve(culledMeshesOut.capacity() +100);
			culledMeshesOut.push_back(OcclusionMeshInfo{*ent,*cmesh});
		}
	});

	auto *pWorld = static_cast<pragma::CWorldComponent*>(c_game->GetWorld());
	if(pWorld != nullptr)
	{
		auto wrldTree = pWorld->GetMeshTree();
		if(wrldTree != nullptr)
		{
			//if(bUpdateLod == true)
			//	entWorld->UpdateLOD(posCam); // TODO: Makes no sense for world geometry?
			// TODO: Assign tree to ModelMesh instead of ModelSubMesh!!!
			auto &entWorld = static_cast<CBaseEntity&>(pWorld->GetEntity());
			auto bViewModel = false;
			std::vector<Plane> *planes = nullptr;
			if(ShouldExamine(renderer,entWorld,bViewModel,cullByViewFrustum ? &planes : nullptr) == true)
			{
				auto &root = wrldTree->GetRootNode();
				auto pTrComponent = entWorld.GetTransformComponent();
				auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
				std::size_t numMeshes = 2; // Value doesn't matter, but has to be > 1
				iterate_occlusion_tree<std::shared_ptr<ModelMesh>>(root,culledMeshesOut,planes,[this,&pos,&bViewModel,&planes,&entWorld,numMeshes,&culledMeshesOut](const std::shared_ptr<ModelMesh> &mesh) {
					auto *cmesh = static_cast<CModelMesh*>(mesh.get());
					if(ShouldExamine(*cmesh,pos,bViewModel,numMeshes,planes) == false)
						return;
					if(culledMeshesOut.capacity() -culledMeshesOut.size() == 0)
						culledMeshesOut.reserve(culledMeshesOut.capacity() +100);
					culledMeshesOut.push_back(OcclusionMeshInfo{entWorld,*cmesh});
				});
			}
		}
	}
}
