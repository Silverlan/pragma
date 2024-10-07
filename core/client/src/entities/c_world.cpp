/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
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
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <cmaterial.h>

import source_engine.bsp;

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void CWorldComponent::Initialize()
{
	BaseWorldComponent::Initialize();

	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		c_game->UpdateEnvironmentLightSource();

		m_lodBaseMeshIds.clear();
		auto &ent = GetEntity();
		auto &mdl = ent.GetModel();
		if(mdl == nullptr) {
			ReloadMeshCache();
			return;
		}
		auto &lods = mdl->GetLODs();
		for(auto &lod : lods) {
			for(auto &pair : lod.meshReplacements)
				m_lodBaseMeshIds[pair.first] = true;
		}
		ReloadMeshCache();
		UpdateRenderMeshes();
	});
	BindEventUnhandled(CPhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		Vector3 min {};
		Vector3 max {};
		auto &ent = GetEntity();
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			pPhysComponent->GetCollisionBounds(&min, &max);
		auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
		if(pRenderComponent.valid())
			pRenderComponent->SetLocalRenderBounds(min, max);
	});
	BindEvent(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> ::util::EventReply {
		BuildOfflineRenderQueues(true);
		return ::util::EventReply::Handled;
	});
#if 0
	BindEventUnhandled(CColorComponent::EVENT_ON_COLOR_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &onColorChangedData = static_cast<pragma::CEOnColorChanged &>(evData.get());
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<CLightDirectionalComponent>>();
		for(auto *ent : entIt) {
			auto pToggleComponent = ent->GetComponent<CToggleComponent>();
			if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn())
				return; // Ambient color already defined by environmental light entity
		}
		c_game->GetWorldEnvironment().SetAmbientColor(onColorChangedData.color.ToVector4());
	});
#endif
	//BindEventUnhandled(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	BuildOfflineRenderQueues(true);
	//});
	auto &ent = GetEntity();
	ent.AddComponent<CBSPComponent>();
	ent.AddComponent<CLightMapComponent>();

	auto mdlC = ent.GetComponent<CModelComponent>();
	if(mdlC.valid())
		mdlC->SetAutoLodEnabled(false);
}
void CWorldComponent::ReloadCHCController()
{
	/*auto &scene = c_game->GetScene();
	auto &cam = scene->GetCamera();
	m_chcController = std::make_shared<CHC>(*cam);
	m_chcController->Reset(m_meshTree);*/ // prosper TODO
}
void CWorldComponent::SetBSPTree(const std::shared_ptr<::util::BSPTree> &bspTree, const std::vector<std::vector<RenderMeshIndex>> &meshesPerCluster)
{
	m_bspTree = bspTree;
	m_meshesPerCluster = meshesPerCluster;
	BuildOfflineRenderQueues(false);
}
const std::shared_ptr<::util::BSPTree> &CWorldComponent::GetBSPTree() const { return m_bspTree; }
void CWorldComponent::ReloadMeshCache()
{
	m_meshTree = nullptr;
	m_chcController = nullptr;
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		return;
	m_meshTree = std::make_shared<OcclusionOctree<std::shared_ptr<ModelMesh>>>(256.f, 1'073'741'824.f, 4096.f, [](const std::weak_ptr<ModelMesh> ptrSubMesh, Vector3 &min, Vector3 &max) {
		if(ptrSubMesh.expired() == true) {
			min = {};
			max = {};
			return;
		}
		auto subMesh = ptrSubMesh.lock();
		subMesh->GetBounds(min, max);
	});
	m_meshTree->Initialize();
	m_meshTree->SetSingleReferenceMode(true);
	m_meshTree->SetToStringCallback([](std::weak_ptr<ModelMesh> ptrMesh) -> std::string {
		if(ptrMesh.expired() == true)
			return "Expired";
		auto subMesh = ptrMesh.lock();
		std::stringstream ss;
		ss << subMesh.get() << " (" << subMesh->GetTriangleCount() << " Tris, " << subMesh->GetVertexCount() << " Vertices)";
		return ss.str();
	});
	// OnUpdateLOD(Vector3{});
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent) {
		for(auto &mesh : pRenderComponent->GetLODMeshes())
			m_meshTree->InsertObject(mesh);
	}
	ReloadCHCController();
}
void CWorldComponent::OnEntitySpawn() { BaseWorldComponent::OnEntitySpawn(); }
void CWorldComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseWorldComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CModelComponent))
		static_cast<CModelComponent &>(component).SetAutoLodEnabled(false);
}
std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> CWorldComponent::GetMeshTree() const { return m_meshTree; };
std::shared_ptr<CHC> CWorldComponent::GetCHCController() const { return m_chcController; }

const pragma::rendering::RenderQueue *CWorldComponent::GetClusterRenderQueue(::util::BSPTree::ClusterIndex clusterIndex, bool translucent) const
{
	auto &queue = translucent ? m_clusterRenderTranslucentQueues : m_clusterRenderQueues;
	return (clusterIndex < queue.size()) ? queue.at(clusterIndex).get() : nullptr;
}

void CWorldComponent::RebuildRenderQueues()
{
	c_game->UpdateEnvironmentLightSource();

	m_lodBaseMeshIds.clear();
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(mdl == nullptr) {
		ReloadMeshCache();
		return;
	}
	auto &lods = mdl->GetLODs();
	for(auto &lod : lods) {
		for(auto &pair : lod.meshReplacements)
			m_lodBaseMeshIds[pair.first] = true;
	}
	ReloadMeshCache();
	UpdateRenderMeshes();
	BuildOfflineRenderQueues(true);
}

#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
void CWorldComponent::BuildOfflineRenderQueues(bool rebuild)
{
	auto &clusterRenderQueues = m_clusterRenderQueues;
	auto &clusterRenderTranslucentQueues = m_clusterRenderTranslucentQueues;
	if(rebuild == false && clusterRenderQueues.empty() == false)
		return;
	for(auto &queue : clusterRenderQueues) {
		if(!queue)
			continue;
		queue->WaitForCompletion();
	}
	for(auto &queue : clusterRenderTranslucentQueues) {
		if(!queue)
			continue;
		queue->WaitForCompletion();
	}
	clusterRenderQueues.clear();
	clusterRenderTranslucentQueues.clear();

	auto &bspTree = GetBSPTree();
	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	if(bspTree == nullptr || renderC.expired() || mdlC.expired())
		return;
	auto &renderMeshes = renderC->GetRenderMeshes();
	auto numClusters = m_bspTree->GetClusterCount();

	std::unordered_map<ModelSubMesh *, ModelMesh *> subMeshToMesh;
	auto &mdl = mdlC->GetModel();
	if(!mdl)
		return;
	for(auto &meshGroup : mdl->GetMeshGroups()) {
		for(auto &mesh : meshGroup->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes())
				subMeshToMesh[subMesh.get()] = mesh.get();
		}
	}

	auto &meshesPerClusters = m_meshesPerCluster;
	if(meshesPerClusters.empty()) {
		meshesPerClusters.resize(numClusters);
		auto fAddClusterMesh = [&meshesPerClusters](::util::BSPTree::ClusterIndex clusterIndex, RenderMeshIndex meshIdx) {
			auto &clusterMeshes = meshesPerClusters.at(clusterIndex);
			if(clusterMeshes.size() == clusterMeshes.capacity())
				clusterMeshes.reserve(clusterMeshes.size() * 1.1 + 100);
			clusterMeshes.push_back(meshIdx);
		};
		for(auto meshIdx = decltype(renderMeshes.size()) {0u}; meshIdx < renderMeshes.size(); ++meshIdx) {
			auto &subMesh = renderMeshes.at(meshIdx);
			auto it = subMeshToMesh.find(subMesh.get());
			if(it == subMeshToMesh.end())
				continue;
			auto *mesh = it->second;
			auto meshClusterIdx = mesh->GetReferenceId();
			if(meshClusterIdx == std::numeric_limits<uint32_t>::max()) {
				// Probably a displacement, which don't have a single cluster associated with them.
				// We'll have to determine which clusters they belong to manually.
				Vector3 min, max;
				mesh->GetBounds(min, max);
				auto leafNodes = m_bspTree->FindLeafNodesInAabb(min, max);
				std::unordered_set<::util::BSPTree::ClusterIndex> clusters;
				for(auto *node : leafNodes) {
					auto meshClusterIdx = node->cluster;
					if(meshClusterIdx == std::numeric_limits<::util::BSPTree::ClusterIndex>::max())
						continue;
					for(auto clusterIdx = decltype(numClusters) {0u}; clusterIdx < numClusters; ++clusterIdx) {
						if(m_bspTree->IsClusterVisible(clusterIdx, meshClusterIdx) == false)
							continue;
						auto it = clusters.find(clusterIdx);
						if(it != clusters.end())
							continue;
						clusters.insert(clusterIdx);
						fAddClusterMesh(clusterIdx, meshIdx);
					}
				}
				continue;
			}
			for(auto clusterIdx = decltype(numClusters) {0u}; clusterIdx < numClusters; ++clusterIdx) {
				if(m_bspTree->IsClusterVisible(clusterIdx, meshClusterIdx) == false)
					continue;
				fAddClusterMesh(clusterIdx, meshIdx);
			}
		}
	}

	clusterRenderQueues.reserve(numClusters);
	clusterRenderTranslucentQueues.reserve(numClusters);
	auto &context = c_engine->GetRenderContext();
	for(auto clusterIdx = decltype(meshesPerClusters.size()) {0u}; clusterIdx < meshesPerClusters.size(); ++clusterIdx) {
		clusterRenderQueues.push_back(pragma::rendering::RenderQueue::Create("world_cluster_" + std::to_string(clusterIdx)));
		std::shared_ptr<pragma::rendering::RenderQueue> clusterRenderTranslucentQueue = nullptr;
		auto &clusterRenderQueue = clusterRenderQueues.back();
		auto &meshes = meshesPerClusters.at(clusterIdx);
		for(auto subMeshIdx : meshes) {
			if(subMeshIdx >= renderMeshes.size()) {
				// Something went wrong (Maybe world model is missing?)
				clusterRenderQueues.clear();
				clusterRenderTranslucentQueues.clear();
				return;
			}
			auto subMesh = renderMeshes.at(subMeshIdx);
			auto *mat = mdlC->GetRenderMaterial(subMesh->GetSkinTextureIndex());
			if(mat == nullptr)
				continue;
			auto hShader = mat->GetPrimaryShader();
			if(!hShader)
				continue;
			auto *shader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(hShader);
			if(shader == nullptr)
				continue;
			uint32_t pipelineIdx = 0;
			auto t = shader->FindPipelineIndex(pragma::rendering::PassType::Generic, renderC->GetShaderPipelineSpecialization(), shader->GetMaterialPipelineSpecializationRequirements(*mat)); // | pragma::GameShaderSpecializationConstantFlag::Enable3dOriginBit);
			if(t.has_value())
				pipelineIdx = *t;
			prosper::PipelineID pipelineId;
			if(shader->GetPipelineId(pipelineId, pipelineIdx) == false || pipelineId == std::numeric_limits<decltype(pipelineId)>::max())
				continue;
			if(mat->GetAlphaMode() == AlphaMode::Blend) {
				clusterRenderTranslucentQueue = clusterRenderTranslucentQueue ? clusterRenderTranslucentQueue : pragma::rendering::RenderQueue::Create("world_translucent_cluster_" + std::to_string(clusterIdx));
				clusterRenderTranslucentQueue->Add(static_cast<CBaseEntity &>(GetEntity()), subMeshIdx, *mat, pipelineId);
				continue;
			}
			clusterRenderQueue->Add(static_cast<CBaseEntity &>(GetEntity()), subMeshIdx, *mat, pipelineId);
		}
		clusterRenderTranslucentQueues.push_back(clusterRenderTranslucentQueue);
		clusterRenderQueue->Sort();
		if(clusterRenderTranslucentQueue)
			clusterRenderTranslucentQueue->Sort();
	}
}

void CWorldComponent::UpdateRenderMeshes()
{
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
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
	for(auto id : baseMeshes) {
		auto it = m_lodBaseMeshIds.find(id);
		if(it == m_lodBaseMeshIds.end()) {
			auto group = mdl->GetMeshGroup(id);
			if(group != nullptr) {
				for(auto &mesh : group->GetMeshes()) {
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
		if(lodInfo != nullptr) {
			auto it = lodInfo->meshReplacements.find(id);
			if(it != lodInfo->meshReplacements.end()) {
				auto lodGroup = mdl->GetMeshGroup(it->second);
				if(lodGroup != nullptr)
					group = lodGroup;
			}
		}
		for(auto &mesh : group->GetMeshes()) {
			lodMeshes.push_back(mesh);
			numSubMeshes += mesh->GetSubMeshCount();
		}
	}
	renderMeshes.reserve(numSubMeshes);
	for(auto &mesh : lodMeshes) {
		for(auto &subMesh : mesh->GetSubMeshes())
			renderMeshes.push_back(subMesh);
	}
}
void CWorldComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////

LINK_ENTITY_TO_CLASS(world, CWorld);

void CWorld::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWorldComponent>();
}

Con::c_cout &CWorld::print(Con::c_cout &os)
{
	os << "CWorld[" << m_index << "]"
	   << "[" << GetClass() << "]"
	   << "[";
	auto &mdl = GetModel();
	if(mdl == nullptr)
		os << "NULL";
	else
		os << mdl->GetName();
	os << "]";
	return os;
}

std::ostream &CWorld::print(std::ostream &os)
{
	os << "CWorld[" << m_index << "]"
	   << "[" << GetClass() << "]"
	   << "[";
	auto &mdl = GetModel();
	if(mdl == nullptr)
		os << "NULL";
	else
		os << mdl->GetName();
	os << "]";
	return os;
}
