/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/c_world.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/physics/collisiontypes.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_bsp_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <util_bsp.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
void CWorldComponent::Initialize()
{
	BaseWorldComponent::Initialize();

	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		c_game->UpdateEnvironmentLightSource();

		m_lodBaseMeshIds.clear();
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr)
		{
			ReloadMeshCache();
			return;
		}
		auto &lods = mdl->GetLODs();
		for(auto &lod : lods)
		{
			for(auto &pair : lod.meshReplacements)
				m_lodBaseMeshIds[pair.first] = true;
		}
		ReloadMeshCache();
		UpdateRenderMeshes();
	});
	BindEventUnhandled(CPhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		Vector3 min {};
		Vector3 max {};
		auto &ent = GetEntity();
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			pPhysComponent->GetCollisionBounds(&min,&max);
		auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
		if(pRenderComponent.valid())
			pRenderComponent->SetRenderBounds(min,max);
	});
	BindEvent(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		BuildOfflineRenderQueues(true);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CColorComponent::EVENT_ON_COLOR_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &onColorChangedData = static_cast<pragma::CEOnColorChanged&>(evData.get());
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<CLightDirectionalComponent>>();
		for(auto *ent : entIt)
		{
			auto pToggleComponent = ent->GetComponent<CToggleComponent>();
			if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn())
				return; // Ambient color already defined by environmental light entity
		}
		c_game->GetWorldEnvironment().SetAmbientColor(onColorChangedData.color.ToVector4());
	});
	//BindEventUnhandled(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	BuildOfflineRenderQueues(true);
	//});
	auto &ent = GetEntity();
	ent.AddComponent<CBSPComponent>();
	ent.AddComponent<CLightMapComponent>();
}
void CWorldComponent::ReloadCHCController()
{
	/*auto &scene = c_game->GetScene();
	auto &cam = scene->GetCamera();
	m_chcController = std::make_shared<CHC>(*cam);
	m_chcController->Reset(m_meshTree);*/ // prosper TODO
}
void CWorldComponent::SetBSPTree(const std::shared_ptr<util::BSPTree> &bspTree,const std::vector<std::vector<RenderMeshIndex>> &meshesPerCluster)
{
	m_bspTree = bspTree;
	m_meshesPerCluster = meshesPerCluster;
	BuildOfflineRenderQueues(false);
}
const std::shared_ptr<util::BSPTree> &CWorldComponent::GetBSPTree() const {return m_bspTree;}
void CWorldComponent::ReloadMeshCache()
{
	m_meshTree = nullptr;
	m_chcController = nullptr;
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	m_meshTree = std::make_shared<OcclusionOctree<std::shared_ptr<ModelMesh>>>(256.f,1'073'741'824.f,4096.f,[](const std::weak_ptr<ModelMesh> ptrSubMesh,Vector3 &min,Vector3 &max) {
		if(ptrSubMesh.expired() == true)
		{
			min = {};
			max = {};
			return;
		}
		auto subMesh = ptrSubMesh.lock();
		subMesh->GetBounds(min,max);
	});
	m_meshTree->Initialize();
	m_meshTree->SetSingleReferenceMode(true);
	m_meshTree->SetToStringCallback([](std::weak_ptr<ModelMesh> ptrMesh) -> std::string {
		if(ptrMesh.expired() == true)
			return "Expired";
		auto subMesh = ptrMesh.lock();
		std::stringstream ss;
		ss<<subMesh.get()<<" ("<<subMesh->GetTriangleCount()<<" Tris, "<<subMesh->GetVertexCount()<<" Vertices)";
		return ss.str();
	});
	// OnUpdateLOD(Vector3{});
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent)
	{
		for(auto &mesh : pRenderComponent->GetLODMeshes())
			m_meshTree->InsertObject(mesh);
	}
	ReloadCHCController();
}
void CWorldComponent::OnEntitySpawn()
{
	BaseWorldComponent::OnEntitySpawn();
}
void CWorldComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseWorldComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CModelComponent))
		static_cast<CModelComponent&>(component).SetAutoLodEnabled(false);
}
std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> CWorldComponent::GetMeshTree() const {return m_meshTree;};
std::shared_ptr<CHC> CWorldComponent::GetCHCController() const {return m_chcController;}

const pragma::rendering::RenderQueue *CWorldComponent::GetClusterRenderQueue(util::BSPTree::ClusterIndex clusterIndex,bool translucent) const
{
	auto &queue = translucent ? m_clusterRenderTranslucentQueues : m_clusterRenderQueues;
	return (clusterIndex < queue.size()) ? queue.at(clusterIndex).get() : nullptr;
}

#include <sharedutils/util_hash.hpp>
void CWorldComponent::BuildOfflineRenderQueues(bool rebuild)
{
	auto &clusterRenderQueues = m_clusterRenderQueues;
	auto &clusterRenderTranslucentQueues = m_clusterRenderTranslucentQueues;
	if(rebuild == false && clusterRenderQueues.empty() == false)
		return;
	clusterRenderQueues.clear();
	clusterRenderTranslucentQueues.clear();

	auto &bspTree = GetBSPTree();
	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	if(bspTree == nullptr || renderC.expired() || mdlC.expired())
		return;
	auto &renderMeshes = renderC->GetRenderMeshes();
	auto numClusters = m_bspTree->GetClusterCount();

	std::unordered_map<ModelSubMesh*,ModelMesh*> subMeshToMesh;
	auto &mdl = mdlC->GetModel();
	for(auto &meshGroup : mdl->GetMeshGroups())
	{
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
				subMeshToMesh[subMesh.get()] = mesh.get();
		}
	}

	auto &meshesPerClusters = m_meshesPerCluster;
	if(meshesPerClusters.empty())
	{
		meshesPerClusters.resize(numClusters);
		auto fAddClusterMesh = [&meshesPerClusters](util::BSPTree::ClusterIndex clusterIndex,RenderMeshIndex meshIdx) {
			auto &clusterMeshes = meshesPerClusters.at(clusterIndex);
			if(clusterMeshes.size() == clusterMeshes.capacity())
				clusterMeshes.reserve(clusterMeshes.size() *1.1 +100);
			clusterMeshes.push_back(meshIdx);
		};
		for(auto meshIdx=decltype(renderMeshes.size()){0u};meshIdx<renderMeshes.size();++meshIdx)
		{
			auto &subMesh = renderMeshes.at(meshIdx);
			auto it = subMeshToMesh.find(subMesh.get());
			if(it == subMeshToMesh.end())
				continue;
			auto *mesh = it->second;
			auto meshClusterIdx = mesh->GetReferenceId();
			if(meshClusterIdx == std::numeric_limits<uint32_t>::max())
			{
				// Probably a displacement, which don't have a single cluster associated with them.
				// We'll have to determine which clusters they belong to manually.
				Vector3 min,max;
				mesh->GetBounds(min,max);
				auto leafNodes = m_bspTree->FindLeafNodesInAABB(min,max);
				std::unordered_set<util::BSPTree::ClusterIndex> clusters;
				for(auto *node : leafNodes)
				{
					auto meshClusterIdx = node->cluster;
					if(meshClusterIdx == std::numeric_limits<util::BSPTree::ClusterIndex>::max())
						continue;
					for(auto clusterIdx=decltype(numClusters){0u};clusterIdx<numClusters;++clusterIdx)
					{
						if(m_bspTree->IsClusterVisible(clusterIdx,meshClusterIdx) == false)
							continue;
						auto it = clusters.find(clusterIdx);
						if(it != clusters.end())
							continue;
						clusters.insert(clusterIdx);
						fAddClusterMesh(clusterIdx,meshIdx);
					}
				}
				continue;
			}
			for(auto clusterIdx=decltype(numClusters){0u};clusterIdx<numClusters;++clusterIdx)
			{
				if(m_bspTree->IsClusterVisible(clusterIdx,meshClusterIdx) == false)
					continue;
				fAddClusterMesh(clusterIdx,meshIdx);
			}
		}
	}

	clusterRenderQueues.reserve(numClusters);
	clusterRenderTranslucentQueues.reserve(numClusters);
	for(auto clusterIdx=decltype(meshesPerClusters.size()){0u};clusterIdx<meshesPerClusters.size();++clusterIdx)
	{
		clusterRenderQueues.push_back(pragma::rendering::RenderQueue::Create());
		std::shared_ptr<pragma::rendering::RenderQueue> clusterRenderTranslucentQueue = nullptr;
		auto &clusterRenderQueue = clusterRenderQueues.back();
		auto &meshes = meshesPerClusters.at(clusterIdx);
		for(auto subMeshIdx : meshes)
		{
			auto subMesh = renderMeshes.at(subMeshIdx);
			auto *mat = mdlC->GetRenderMaterial(subMesh->GetSkinTextureIndex());
			if(mat == nullptr)
				continue;
			auto hShader = mat->GetPrimaryShader();
			if(hShader.expired())
				continue;
			auto *shader = dynamic_cast<pragma::ShaderTextured3DBase*>(hShader.get());
			if(shader == nullptr)
				continue;
			if(mat->GetAlphaMode() != AlphaMode::Opaque)
			{
				clusterRenderTranslucentQueue = clusterRenderTranslucentQueue ? clusterRenderTranslucentQueue : pragma::rendering::RenderQueue::Create();
				clusterRenderTranslucentQueue->Add(static_cast<CBaseEntity&>(GetEntity()),subMeshIdx,*mat,*shader);
				continue;
			}
			clusterRenderQueue->Add(static_cast<CBaseEntity&>(GetEntity()),subMeshIdx,*mat,*shader);
		}
		clusterRenderTranslucentQueues.push_back(clusterRenderTranslucentQueue);
		clusterRenderQueue->Sort();
		if(clusterRenderTranslucentQueue)
			clusterRenderTranslucentQueue->Sort();
	}

	if(true)
		return;
	// TODO: Remove this block
	// Test
	struct ClusterSharedMeshPair
	{
		pragma::RenderMeshIndex mesh0 = 0;
		pragma::RenderMeshIndex mesh1 = 0;
		//uint32_t numClustersWithEitherMesh = 0;
		uint32_t numClusters = 0;
		// Describes the percentage of clusters where both meshes are visible
		// in relation to the number of clusters where either mesh is visible
		float clusterVisibilityPercentage = 0.f;
	};
	std::unordered_map<uint64_t,ClusterSharedMeshPair> clusterMeshPairs;
	clusterMeshPairs.reserve(numClusters *numClusters);

	auto fFindMeshInCluster = [&meshesPerClusters](RenderMeshIndex idx) -> std::optional<uint32_t> {
		for(auto icluster=decltype(meshesPerClusters.size()){0u};icluster<meshesPerClusters.size();++icluster)
		{
			auto &meshList = meshesPerClusters.at(icluster);
			auto it = std::find(meshList.begin(),meshList.end(),idx);
			if(it == meshList.end())
				continue;
			return icluster;
		}
		return {};
	};

	std::cout<<"Found 0: "<<*fFindMeshInCluster(0)<<std::endl;
	std::cout<<"Found 830: "<<*fFindMeshInCluster(830)<<std::endl;

	Con::cout<<"Cluster count: "<<numClusters<<Con::endl;
	auto fGetMeshPairHash = [](pragma::RenderMeshIndex mesh0,pragma::RenderMeshIndex mesh1) {
		auto first = (mesh0 < mesh1) ? mesh0 : mesh1;
		auto second = (first != mesh0) ? mesh0 : mesh1;
		auto hash = util::hash_combine(0,first);
		return util::hash_combine(hash,second);
	};
	std::vector<std::vector<util::BSPTree::ClusterIndex>> meshToClusters;
	meshToClusters.resize(renderMeshes.size());
	for(auto clusterIdx=decltype(meshesPerClusters.size()){0u};clusterIdx<meshesPerClusters.size();++clusterIdx)
	{
		auto &clusterMeshIndices = meshesPerClusters.at(clusterIdx);
		for(auto meshIdx : clusterMeshIndices)
			meshToClusters.at(meshIdx).push_back(clusterIdx);
	}
	for(auto meshIdx0=decltype(renderMeshes.size()){0u};meshIdx0<renderMeshes.size();++meshIdx0)
	{
		auto &mesh0 = renderMeshes.at(meshIdx0);
		auto &clusters0 = meshToClusters.at(meshIdx0);
		for(auto meshIdx1=meshIdx0 +1;meshIdx1<renderMeshes.size();++meshIdx1)
		{
			auto &mesh1 = renderMeshes.at(meshIdx1);
			if(mesh1->GetSkinTextureIndex() != mesh0->GetSkinTextureIndex())
				continue;
			auto &clusters1 = meshToClusters.at(meshIdx1);
			auto numClustersWithSharedVisibility = std::count_if(clusters0.begin(),clusters0.end(),[&clusters1,&bspTree](util::BSPTree::ClusterIndex cluster0) {
				return std::find_if(clusters1.begin(),clusters1.end(),[&bspTree,cluster0](util::BSPTree::ClusterIndex cluster1) {return bspTree->IsClusterVisible(cluster0,cluster1);}) != clusters1.end();
			});

			if(numClustersWithSharedVisibility == 0)
				continue;
			auto hash = fGetMeshPairHash(meshIdx0,meshIdx1);
			auto it = clusterMeshPairs.find(hash);
			if(it != clusterMeshPairs.end())
				std::cout<<"ERR";
			it = clusterMeshPairs.insert(std::make_pair(hash,ClusterSharedMeshPair{})).first;
			it->second.mesh0 = meshIdx0;
			it->second.mesh1 = meshIdx1;
			it->second.numClusters = numClustersWithSharedVisibility;
		}
	}
#if 0
	for(auto i=decltype(numClusters){0u};i<numClusters;++i)
	{
		auto &clusterMeshIndices = meshesPerClusters.at(i);
		for(auto j=decltype(clusterMeshIndices.size()){0u};j<clusterMeshIndices.size();++j)
		{
			auto subMeshIdx0 = clusterMeshIndices.at(j);
			numClustersPerMesh.at(subMeshIdx0)++;

			auto &subMesh0 = renderMeshes.at(subMeshIdx0);
			for(auto k=j +1;k<clusterMeshIndices.size();++k)
			{
				auto subMeshIdx1 = clusterMeshIndices.at(k);
				if((subMeshIdx0 == 0 && subMeshIdx1 == 830) || (subMeshIdx0 == 830 && subMeshIdx1 == 0))
					std::cout<<"";
				auto &subMesh1 = renderMeshes.at(subMeshIdx1);
				if(subMesh1->GetSkinTextureIndex() != subMesh0->GetSkinTextureIndex())
					continue;
				auto first = (subMeshIdx0 < subMeshIdx1) ? subMeshIdx0 : subMeshIdx1;
				auto second = (first == subMeshIdx0) ? subMeshIdx1 : subMeshIdx0;
				auto hash = fGetMeshPairHash(first,second);
				auto it = clusterMeshPairs.find(hash);
				if(it == clusterMeshPairs.end())
				{
					it = clusterMeshPairs.insert(std::make_pair(hash,ClusterSharedMeshPair{})).first;
					it->second.mesh0 = first;
					it->second.mesh1 = second;
				}
				++it->second.numClusters;
			}
		}


		Con::cout<<"Cluster: "<<i<<Con::endl;
		Con::cout<<"Mesh count: "<<meshesPerClusters.at(i).size()<<Con::endl;
		uint32_t numVerts = 0;
		uint32_t numTris = 0;
		for(auto &subMeshIdx : meshesPerClusters.at(i))
		{
			auto &subMesh = renderMeshes.at(subMeshIdx);
			numVerts += subMesh->GetVertexCount();
			numTris += subMesh->GetTriangleCount();
		}
		Con::cout<<"Triangle count: "<<numTris<<Con::endl;
		Con::cout<<Con::endl;
	}
	Con::cout<<"";
#endif


	for(auto &pair : clusterMeshPairs)
	{
		auto max = umath::max(meshToClusters.at(pair.second.mesh0).size(),meshToClusters.at(pair.second.mesh1).size());
		pair.second.clusterVisibilityPercentage = pair.second.numClusters /static_cast<float>(max);
		if(pair.second.clusterVisibilityPercentage > 1.f)
			std::cout<<"ERR";
	}


	std::vector<uint64_t> sortedClusterMeshPairs;
	sortedClusterMeshPairs.reserve(clusterMeshPairs.size());
	for(auto &pair : clusterMeshPairs)
		sortedClusterMeshPairs.push_back(pair.first);

	std::sort(sortedClusterMeshPairs.begin(),sortedClusterMeshPairs.end(),[&clusterMeshPairs](size_t hash0,size_t hash1) {
		return clusterMeshPairs[hash0].clusterVisibilityPercentage > clusterMeshPairs[hash1].clusterVisibilityPercentage;
	});

	const auto threshold = 0.33f;
	using MeshMergeIndex = uint32_t;
	std::vector<std::vector<pragma::RenderMeshIndex>> mergeInfoTable;
	std::unordered_map<pragma::RenderMeshIndex,MeshMergeIndex> meshMergeList;
	for(auto idx : sortedClusterMeshPairs)
	{
		auto &pair = clusterMeshPairs[idx];
		if(pair.clusterVisibilityPercentage < threshold)
			break;
		auto it0 = meshMergeList.find(pair.mesh0);
		auto it1 = meshMergeList.find(pair.mesh1);
		if(it0 != meshMergeList.end() && it1 != meshMergeList.end())
			continue; // Both meshes are already merged with others
		if(it0 == meshMergeList.end() && it1 == meshMergeList.end())
		{
			// Neither mesh has been merged with anything, mark them as
			// a new merge pair
			auto idx = mergeInfoTable.size();
			meshMergeList[pair.mesh0] = idx;
			meshMergeList[pair.mesh1] = idx;
			mergeInfoTable.push_back({pair.mesh0,pair.mesh1});
			continue;
		}
		// Either of the meshes has already been merged with another
		auto itDst = (it0 != meshMergeList.end()) ? it0 : it1;
		auto meshIdxSrc = (it0 != meshMergeList.end()) ? pair.mesh1 : pair.mesh0;
		auto mergeIdx = itDst->second;
		auto &meshIndices = mergeInfoTable.at(mergeIdx);
		auto isMeshVisibilityWithinThreshold = true;
		for(auto meshIdx : meshIndices)
		{
			auto hash = fGetMeshPairHash(meshIdx,meshIdxSrc);
			auto it = clusterMeshPairs.find(hash);
			if(it == clusterMeshPairs.end())
			{
				// Meshes have no shared visibility at all
				isMeshVisibilityWithinThreshold = false;
				break;
			}
			auto &pairInfo = it->second;
			if(pairInfo.clusterVisibilityPercentage >= threshold)
				continue;
			isMeshVisibilityWithinThreshold = false;
			break;
		}
		if(isMeshVisibilityWithinThreshold == false)
			continue; // Don't merge
		meshMergeList[meshIdxSrc] = mergeIdx;
		mergeInfoTable.at(mergeIdx).push_back(meshIdxSrc);
	}

	Con::cout<<"Original mesh count: "<<renderMeshes.size()<<Con::endl;
	Con::cout<<"Number of meshes that will be merged: "<<meshMergeList.size()<<Con::endl;
	Con::cout<<"Number of meshes after merging: "<<(mergeInfoTable.size() +(renderMeshes.size() -meshMergeList.size()))<<Con::endl;

	for(auto &mergeList : mergeInfoTable)
	{
		if(mergeList.size() <= 1)
			continue;
		auto meshIdxDst = mergeList.front();
		auto &meshDst = renderMeshes.at(meshIdxDst);
		for(auto it=mergeList.begin() +1;it<mergeList.end();++it)
		{
			auto meshIdxSrc = *it;
			auto &meshSrc = renderMeshes.at(meshIdxSrc);
			meshDst->Merge(*meshSrc);
		}
	}
}

void CWorldComponent::UpdateRenderMeshes()
{
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto mdl = ent.GetModel();
	auto pRenderComponent = ent.GetRenderComponent();
	if(mdl == nullptr || !pRenderComponent)
		return;
	auto &baseMeshes = mdl->GetBaseMeshes();
	auto &lodMeshes = pRenderComponent->GetLODMeshes();
	auto &renderMeshes = pRenderComponent->GetRenderMeshes();
	lodMeshes.clear();
	renderMeshes.clear();
	uint32_t numSubMeshes = 0;
	for(auto id : baseMeshes)
	{
		auto it = m_lodBaseMeshIds.find(id);
		if(it == m_lodBaseMeshIds.end())
		{
			auto group = mdl->GetMeshGroup(id);
			if(group != nullptr)
			{
				for(auto &mesh : group->GetMeshes())
				{
					lodMeshes.push_back(mesh);
					numSubMeshes += mesh->GetSubMeshCount();
				}
			}
			continue;
		}
		auto group = mdl->GetMeshGroup(id);
		if(group == nullptr)
			continue;
		auto &meshes = group->GetMeshes();
		if(meshes.empty())
			continue;
		auto &mesh = meshes.front();
		auto *lodInfo = mdl->GetLODInfo(0);
		if(lodInfo != nullptr)
		{
			auto it = lodInfo->meshReplacements.find(id);
			if(it != lodInfo->meshReplacements.end())
			{
				auto lodGroup = mdl->GetMeshGroup(it->second);
				if(lodGroup != nullptr)
					group = lodGroup;
			}
		}
		for(auto &mesh : group->GetMeshes())
		{
			lodMeshes.push_back(mesh);
			numSubMeshes += mesh->GetSubMeshCount();
		}
	}
	renderMeshes.reserve(numSubMeshes);
	for(auto &mesh : lodMeshes)
	{
		for(auto &subMesh : mesh->GetSubMeshes())
			renderMeshes.push_back(subMesh);
	}
}
luabind::object CWorldComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CWorldComponentHandleWrapper>(l);}

//////////////

LINK_ENTITY_TO_CLASS(world,CWorld);

void CWorld::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWorldComponent>();
}

Con::c_cout& CWorld::print(Con::c_cout &os)
{
	os<<"CWorld["<<m_index<<"]"<<"["<<GetClass()<<"]"<<"[";
	auto mdlComponent = GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		os<<"NULL";
	else
		os<<mdl->GetName();
	os<<"]";
	return os;
}

std::ostream& CWorld::print(std::ostream &os)
{
	os<<"CWorld["<<m_index<<"]"<<"["<<GetClass()<<"]"<<"[";
	auto mdlComponent = GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		os<<"NULL";
	else
		os<<mdl->GetName();
	os<<"]";
	return os;
}
#pragma optimize("",on)
