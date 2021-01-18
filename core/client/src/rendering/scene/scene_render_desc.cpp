/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_bsp.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_chc.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_inert.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_octtree.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/renderers/base_renderer.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/render_queue_worker.hpp"
#include "pragma/rendering/render_queue_instancer.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include <sharedutils/util_shaderinfo.hpp>
#include <sharedutils/alpha_mode.hpp>
#include <sharedutils/util_hash.hpp>
#include <pragma/entities/entity_iterator.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
SceneRenderDesc::SceneRenderDesc(pragma::CSceneComponent &scene)
	: m_scene{scene}
{
	for(auto &renderQueue : m_renderQueues)
		renderQueue = pragma::rendering::RenderQueue::Create();
	ReloadOcclusionCullingHandler();
}
SceneRenderDesc::~SceneRenderDesc()
{
	m_occlusionCullingHandler = nullptr;
}

const std::vector<pragma::OcclusionMeshInfo> &SceneRenderDesc::GetCulledMeshes() const {return m_renderMeshCollectionHandler.GetOcclusionFilteredMeshes();}
std::vector<pragma::OcclusionMeshInfo> &SceneRenderDesc::GetCulledMeshes() {return m_renderMeshCollectionHandler.GetOcclusionFilteredMeshes();}
const std::vector<pragma::CParticleSystemComponent*> &SceneRenderDesc::GetCulledParticles() const {return m_renderMeshCollectionHandler.GetOcclusionFilteredParticleSystems();}
std::vector<pragma::CParticleSystemComponent*> &SceneRenderDesc::GetCulledParticles() {return m_renderMeshCollectionHandler.GetOcclusionFilteredParticleSystems();}

const pragma::OcclusionCullingHandler &SceneRenderDesc::GetOcclusionCullingHandler() const {return const_cast<SceneRenderDesc*>(this)->GetOcclusionCullingHandler();}
pragma::OcclusionCullingHandler &SceneRenderDesc::GetOcclusionCullingHandler() {return *m_occlusionCullingHandler;}
void SceneRenderDesc::SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler) {m_occlusionCullingHandler = handler;}
void SceneRenderDesc::SetOcclusionCullingMethod(OcclusionCullingMethod method)
{
	switch(method)
	{
	case OcclusionCullingMethod::BruteForce: /* Brute-force */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerBruteForce>();
		break;
	case OcclusionCullingMethod::CHCPP: /* CHC++ */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerCHC>();
		break;
	case OcclusionCullingMethod::BSP: /* BSP */
	{
		auto *world = c_game->GetWorld();
		if(world)
		{
			auto &entWorld = world->GetEntity();
			auto pWorldComponent = entWorld.GetComponent<pragma::CWorldComponent>();
			auto bspTree = pWorldComponent.valid() ? pWorldComponent->GetBSPTree() : nullptr;
			if(bspTree != nullptr && bspTree->GetNodes().size() > 1u)
			{
				m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerBSP>(bspTree);
				break;
			}
		}
	}
	case OcclusionCullingMethod::Octree: /* Octtree */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerOctTree>();
		break;
	case OcclusionCullingMethod::Inert: /* Off */
	default:
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerInert>();
		break;
	}
	m_occlusionCullingHandler->Initialize();
}
void SceneRenderDesc::ReloadOcclusionCullingHandler()
{
	auto occlusionCullingMode = static_cast<OcclusionCullingMethod>(c_game->GetConVarInt("cl_render_occlusion_culling"));
	SetOcclusionCullingMethod(occlusionCullingMode);
}

static auto cvDrawGlow = GetClientConVar("render_draw_glow");
static auto cvDrawTranslucent = GetClientConVar("render_draw_translucent");
static auto cvDrawSky = GetClientConVar("render_draw_sky");
static auto cvDrawWater = GetClientConVar("render_draw_water");
static auto cvDrawView = GetClientConVar("render_draw_view");
pragma::rendering::CulledMeshData *SceneRenderDesc::GetRenderInfo(RenderMode renderMode) const
{
	auto &renderMeshData = m_renderMeshCollectionHandler.GetRenderMeshData();
	auto it = renderMeshData.find(renderMode);
	if(it == renderMeshData.end())
		return nullptr;
	return it->second.get();
}

pragma::rendering::RenderMeshCollectionHandler &SceneRenderDesc::GetRenderMeshCollectionHandler() {return m_renderMeshCollectionHandler;}
const pragma::rendering::RenderMeshCollectionHandler &SceneRenderDesc::GetRenderMeshCollectionHandler() const {return const_cast<SceneRenderDesc*>(this)->GetRenderMeshCollectionHandler();}

static FRender render_mode_to_render_flag(RenderMode renderMode)
{
	switch(renderMode)
	{
	case RenderMode::World:
		return FRender::World;
	case RenderMode::View:
		return FRender::View;
	case RenderMode::Skybox:
		return FRender::Skybox;
	case RenderMode::Water:
		return FRender::Water;
	}
	return FRender::None;
}

SceneRenderDesc::RenderQueueId SceneRenderDesc::GetRenderQueueId(RenderMode renderMode,bool translucent) const
{
	switch(renderMode)
	{
	case RenderMode::Skybox:
		return !translucent ? RenderQueueId::Skybox : RenderQueueId::SkyboxTranslucent;
	case RenderMode::View:
		return !translucent ? RenderQueueId::View : RenderQueueId::ViewTranslucent;
	case RenderMode::Water:
		return !translucent ? RenderQueueId::Water : RenderQueueId::Invalid;
	case RenderMode::World:
		return !translucent ? RenderQueueId::World : RenderQueueId::WorldTranslucent;
	}
	static_assert(umath::to_integral(RenderQueueId::Count) == 7u);
	return RenderQueueId::Invalid;
}
pragma::rendering::RenderQueue *SceneRenderDesc::GetRenderQueue(RenderMode renderMode,bool translucent)
{
	auto renderQueueId = GetRenderQueueId(renderMode,translucent);
	return (renderQueueId != RenderQueueId::Invalid) ? m_renderQueues.at(umath::to_integral(renderQueueId)).get() : nullptr;
}
const pragma::rendering::RenderQueue *SceneRenderDesc::GetRenderQueue(RenderMode renderMode,bool translucent) const
{
	return const_cast<SceneRenderDesc*>(this)->GetRenderQueue(renderMode,translucent);
}
const std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> &SceneRenderDesc::GetWorldRenderQueues() const {return m_worldRenderQueues;}

void SceneRenderDesc::AddRenderMeshesToRenderQueue(
	const util::DrawSceneInfo &drawSceneInfo,pragma::CRenderComponent &renderC,
	const std::function<pragma::rendering::RenderQueue*(RenderMode,bool)> &getRenderQueue,
	const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull,
	int32_t lodBias,const std::function<void(pragma::rendering::RenderQueue&,const pragma::rendering::RenderQueueItem&)> &fOptInsertItemToQueue
)
{
	auto *mdlC = renderC.GetModelComponent();
	auto lod = umath::max(static_cast<int32_t>(mdlC->GetLOD()) +lodBias,0);
	auto &renderMeshes = renderC.GetRenderMeshes();
	auto &renderBufferData = renderC.GetRenderBufferData();
	auto &lodGroup = renderC.GetLodRenderMeshGroup(lod);
	renderC.UpdateRenderDataMT(drawSceneInfo.commandBuffer,scene,cam,vp);
	auto renderMode = renderC.GetRenderMode();
	auto *renderer = drawSceneInfo.scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto &context = c_engine->GetRenderContext();
	auto renderTranslucent = umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Translucent);
	auto *rasterizationRenderer = static_cast<const pragma::rendering::RasterizationRenderer*>(renderer);
	for(auto meshIdx=lodGroup.first;meshIdx<lodGroup.first +lodGroup.second;++meshIdx)
	{
		if(fShouldCull && ShouldCull(renderC,meshIdx,fShouldCull))
			continue;
		auto &renderMesh = renderMeshes[meshIdx];
		auto *mat = mdlC->GetRenderMaterial(renderMesh->GetSkinTextureIndex());
		if(mat == nullptr)
			continue;
		auto *shader = static_cast<pragma::ShaderGameWorldLightingPass*>(mat->GetUserData2());
		if(shader == nullptr)
		{
			// TODO: Shaders are initialized lazily and calling GetPrimaryShader may invoke the load process.
			// This is illegal here, since this function may be called from a thread other than the main thread!
			shader = dynamic_cast<pragma::ShaderGameWorldLightingPass*>(mat->GetPrimaryShader());
			mat->SetUserData2(shader); // TODO: This is technically *not* thread safe and could be called from multiple threads!
		}
		shader = rasterizationRenderer->GetShaderOverride(shader);
		if(shader == nullptr)
			continue;
		auto pipelineIdx = shader->FindPipelineIndex(
			pragma::ShaderGameWorldLightingPass::PassType::Generic,
			renderC.GetShaderPipelineSpecialization(),
			renderBufferData[meshIdx].pipelineSpecializationFlags
		);
		prosper::PipelineID pipelineId;
		if(pipelineIdx.has_value() == false || shader->GetPipelineId(pipelineId,*pipelineIdx) == false)
			continue;
		// shader = rasterizationRenderer->GetShaderOverride(shader);
		// if(shader == nullptr)
		// 	continue;
		auto translucent = mat->GetAlphaMode() == AlphaMode::Blend;
		if(renderTranslucent == false && translucent)
			continue;
		auto *renderQueue = getRenderQueue(renderMode,translucent);
		if(renderQueue == nullptr)
			continue;
		pragma::rendering::RenderQueueItem item {static_cast<CBaseEntity&>(renderC.GetEntity()),meshIdx,*mat,pipelineId,translucent ? &cam : nullptr};
		if(fOptInsertItemToQueue)
			fOptInsertItemToQueue(*renderQueue,item);
		else
			renderQueue->Add(item);
	}
}
void SceneRenderDesc::AddRenderMeshesToRenderQueue(
	const util::DrawSceneInfo &drawSceneInfo,pragma::CRenderComponent &renderC,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,
	const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull
)
{
	AddRenderMeshesToRenderQueue(drawSceneInfo,renderC,[this](RenderMode renderMode,bool translucent) {return GetRenderQueue(renderMode,translucent);},scene,cam,vp,fShouldCull);
}

bool SceneRenderDesc::ShouldCull(CBaseEntity &ent,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull)
{
	auto *renderC = ent.GetRenderComponent();
	return !renderC || ShouldCull(*renderC,fShouldCull);
}
bool SceneRenderDesc::ShouldCull(pragma::CRenderComponent &renderC,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull)
{
	auto &aabb = renderC.GetAbsoluteRenderBounds();
	return fShouldCull(aabb.min,aabb.max);
}
bool SceneRenderDesc::ShouldCull(pragma::CRenderComponent &renderC,pragma::RenderMeshIndex meshIdx,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull)
{
	auto &renderMeshes = renderC.GetRenderMeshes();
	if(meshIdx >= renderMeshes.size())
		return false;
	auto &renderMesh = renderMeshes[meshIdx];
	Vector3 min,max;
	renderMesh->GetBounds(min,max);
	auto &pos = renderC.GetEntity().GetPosition();
	min += pos;
	max += pos;
	return fShouldCull(min,max);
}
bool SceneRenderDesc::ShouldCull(const Vector3 &min,const Vector3 &max,const std::vector<umath::Plane> &frustumPlanes)
{
	return umath::intersection::aabb_in_plane_mesh(min,max,frustumPlanes) == umath::intersection::Intersect::Outside;
}

static auto cvEntitiesPerJob = GetClientConVar("render_queue_entities_per_worker_job");
void SceneRenderDesc::CollectRenderMeshesFromOctree(
	const util::DrawSceneInfo &drawSceneInfo,const OcclusionOctree<CBaseEntity*> &tree,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,FRender renderFlags,
	const std::function<pragma::rendering::RenderQueue*(RenderMode,bool)> &getRenderQueue,
	const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull,const std::vector<util::BSPTree*> *bspTrees,const std::vector<util::BSPTree::Node*> *bspLeafNodes,
	int32_t lodBias,const std::function<bool(CBaseEntity&,const pragma::CSceneComponent&,FRender)> &shouldConsiderEntity
)
{
	auto numEntitiesPerWorkerJob = umath::max(cvEntitiesPerJob->GetInt(),1);
	std::function<void(const OcclusionOctree<CBaseEntity*>::Node &node)> iterateTree = nullptr;
	iterateTree = [&iterateTree,&shouldConsiderEntity,&scene,&cam,renderFlags,fShouldCull,&drawSceneInfo,&getRenderQueue,&vp,bspLeafNodes,bspTrees,lodBias,numEntitiesPerWorkerJob](const OcclusionOctree<CBaseEntity*>::Node &node) {
		auto &nodeBounds = node.GetWorldBounds();
		if(fShouldCull && fShouldCull(nodeBounds.first,nodeBounds.second))
			return;
		if(bspLeafNodes && bspLeafNodes->empty() == false)
		{
			auto hasIntersection = false;
			for(auto i=decltype(bspLeafNodes->size()){0u};i<bspLeafNodes->size();++i)
			{
				auto *node = (*bspLeafNodes)[i];
				if(umath::intersection::aabb_aabb(nodeBounds.first,nodeBounds.second,node->minVisible,node->maxVisible) == umath::intersection::Intersect::Outside)
					continue;
				if((*bspTrees)[i]->IsAABBVisibleInCluster(nodeBounds.first,nodeBounds.second,node->cluster) == false)
					continue;
				hasIntersection = true;
				break;
			}
			if(hasIntersection == false)
				return;
		}
		auto &objs = node.GetObjects();
		auto numObjects = objs.size();
		auto numBatches = (numObjects /numEntitiesPerWorkerJob) +(((numObjects %numEntitiesPerWorkerJob) > 0) ? 1 : 0);
		for(auto i=decltype(numBatches){0u};i<numBatches;++i)
		{
			auto iStart = i *numEntitiesPerWorkerJob;
			auto iEnd = umath::min(static_cast<size_t>(iStart +numEntitiesPerWorkerJob),numObjects);
			c_game->GetRenderQueueWorkerManager().AddJob([iStart,iEnd,&drawSceneInfo,shouldConsiderEntity,&objs,renderFlags,getRenderQueue,&scene,&cam,vp,fShouldCull,lodBias]() {
				// Note: We don't add individual items directly to the render queue, because that would invoke
				// a mutex lock which can stall all of the worker threads.
				// Instead we'll collect the entire batch of items, then add all of them to the render queue at once.
				std::unordered_map<pragma::rendering::RenderQueue*,std::vector<pragma::rendering::RenderQueueItem>> items;
				for(auto i=iStart;i<iEnd;++i)
				{
					auto *ent = objs[i];
					assert(ent);
					if(ent == nullptr)
					{
						// This should NEVER occur, but seems to anyway in some rare cases
						Con::cerr<<"ERROR: NULL Entity in dynamic scene occlusion octree! Ignoring..."<<Con::endl;
						continue;
					}

					if(ent->IsWorld())
						continue; // World entities are handled separately
					auto *renderC = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
					if(
						!renderC || renderC->IsExemptFromOcclusionCulling() || ShouldConsiderEntity(*static_cast<CBaseEntity*>(ent),scene,renderFlags) == false || 
						(shouldConsiderEntity && shouldConsiderEntity(*static_cast<CBaseEntity*>(ent),scene,renderFlags) == false) //||
						//(camClusterIdx.has_value() && renderC->IsVisibleInCluster(*camClusterIdx) == false)
					)
						continue;
					if(fShouldCull && ShouldCull(*renderC,fShouldCull))
						continue;
					AddRenderMeshesToRenderQueue(drawSceneInfo,*renderC,getRenderQueue,scene,cam,vp,fShouldCull,lodBias,[&items](pragma::rendering::RenderQueue &renderQueue,const pragma::rendering::RenderQueueItem &item) {
						auto &v = items[&renderQueue];
						if(v.size() == v.capacity())
							v.reserve(v.size() *1.1f +50);
						v.push_back(item);
					});
				}
				for(auto &pair : items)
					pair.first->Add(pair.second);
			});
		}
		auto *children = node.GetChildren();
		if(children == nullptr)
			return;
		for(auto &c : *children)
		{
			if(!c)
				continue;
			iterateTree(static_cast<OcclusionOctree<CBaseEntity*>::Node&>(*c));
		}
	};
	iterateTree(tree.GetRootNode());
}
void SceneRenderDesc::CollectRenderMeshesFromOctree(
	const util::DrawSceneInfo &drawSceneInfo,const OcclusionOctree<CBaseEntity*> &tree,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,FRender renderFlags,
	const std::vector<umath::Plane> &frustumPlanes,const std::vector<util::BSPTree*> *bspTrees,const std::vector<util::BSPTree::Node*> *bspLeafNodes
)
{
	CollectRenderMeshesFromOctree(drawSceneInfo,tree,scene,cam,vp,renderFlags,[this](RenderMode renderMode,bool translucent) {return GetRenderQueue(renderMode,translucent);},
	[&frustumPlanes](const Vector3 &min,const Vector3 &max) -> bool {
		return umath::intersection::aabb_in_plane_mesh(min,max,frustumPlanes) == umath::intersection::Intersect::Outside;
	},bspTrees,bspLeafNodes,0,nullptr);
}

bool SceneRenderDesc::ShouldConsiderEntity(CBaseEntity &ent,const pragma::CSceneComponent &scene,FRender renderFlags)
{
	if(ent.IsInScene(scene) == false || !ent.GetRenderComponent())
		return false;
	auto *renderC = ent.GetRenderComponent();
	auto renderMode = renderC->GetRenderMode();
	return umath::is_flag_set(renderFlags,render_mode_to_render_flag(renderMode)) && ent.GetModel() != nullptr && renderC->ShouldDraw();
}

struct DebugFreezeCamData
{
	Vector3 pos;
	std::vector<umath::Plane> frustumPlanes;
};
static std::optional<DebugFreezeCamData> g_debugFreezeCamData = {};
static void cmd_debug_occlusion_culling_freeze_camera(NetworkState*,ConVar*,bool,bool val)
{
	g_debugFreezeCamData = {};
	if(val == false)
		return;
	auto *scene = c_game->GetRenderScene();
	if(scene == nullptr)
		return;
	auto &cam = scene->GetActiveCamera();
	if(cam.expired())
		return;
	g_debugFreezeCamData = DebugFreezeCamData{};
	g_debugFreezeCamData->pos = cam->GetEntity().GetPosition();
	g_debugFreezeCamData->frustumPlanes = cam->GetFrustumPlanes();
}
REGISTER_CONVAR_CALLBACK_CL(debug_occlusion_culling_freeze_camera,cmd_debug_occlusion_culling_freeze_camera);

bool SceneRenderDesc::IsWorldMeshVisible(uint32_t worldRenderQueueIndex,pragma::RenderMeshIndex meshIdx) const
{
	if(worldRenderQueueIndex >= m_worldMeshVisibility.size())
		return false;
	auto &worldMeshVisibility = m_worldMeshVisibility.at(worldRenderQueueIndex);
	return (meshIdx < worldMeshVisibility.size()) ? worldMeshVisibility.at(meshIdx) : false;
}

void SceneRenderDesc::WaitForWorldRenderQueues() const {while(m_worldRenderQueuesReady == false);}

static auto cvInstancingEnabled = GetClientConVar("render_instancing_enabled");
void SceneRenderDesc::BuildRenderQueueInstanceLists(pragma::rendering::RenderQueue &renderQueue)
{
	renderQueue.instanceSets.clear();
	if(cvInstancingEnabled->GetBool() == false)
		return;
	pragma::rendering::RenderQueueInstancer instancer {renderQueue};
	instancer.Process();

#if 0
	util::Hash prevEntityHash = 0;
	util::Hash curEntityHash = 0;
	uint32_t numHashMatches = 0;
	auto instanceThreshold = umath::max(cvInstancingThreshold->GetInt(),2);
	std::vector<EntityIndex> instantiableEntityList;
	auto fUpdateEntityInstanceLists = [&instantiableEntityList,&curEntityHash,&prevEntityHash,instanceThreshold](
		pragma::rendering::RenderQueue &renderQueue,EntityIndex entIdx,uint32_t numMeshes,pragma::rendering::RenderQueueItemSortPair *sortItem
	) {
		// New entity
		auto *ent = static_cast<CBaseEntity*>(c_game->GetEntityByLocalIndex(entIdx));
		auto *renderC = ent ? ent ->GetRenderComponent() : nullptr;
		if(instantiableEntityList.size() > 1 && curEntityHash != prevEntityHash)
		{
			// Last entity has a different hash than the one before; We'll close the instantiation list and open a new one
			auto numInstantiable = instantiableEntityList.size() -1;
			if(numInstantiable < instanceThreshold)
				instantiableEntityList.erase(instantiableEntityList.begin(),instantiableEntityList.begin() +(numInstantiable -1));
			else
			{
				auto newItem = instantiableEntityList.back(); // NOT part of our instance list!
				instantiableEntityList.erase(instantiableEntityList.end() -1);

				// [0,#instantiableEntityList) can be instanced
				std::vector<pragma::RenderBufferIndex> renderBufferIndices {};
				renderBufferIndices.reserve(instantiableEntityList.size());
				for(auto &entIdx : instantiableEntityList)
				{
					auto renderBufferIndex = static_cast<CBaseEntity*>(c_game->GetEntityByLocalIndex(entIdx))->GetRenderComponent()->GetRenderBufferIndex();
					renderBufferIndices.push_back(*renderBufferIndex);
				}
				auto &instanceIndexBuffer = pragma::CSceneComponent::GetEntityInstanceIndexBuffer();
				auto numInstances = renderBufferIndices.size();
				auto instanceBuf = instanceIndexBuffer->AddInstanceList(renderQueue,std::move(renderBufferIndices),util::hash_combine<uint64_t>(prevEntityHash,numInstances));
				// We'll iterate backwards through the sorted render queue and mark the
				// items as instanced.
				for(auto i=decltype(numInstances){0u};i<(numInstances -1);++i)
				{
					// The renderer will know to skip all items mark as 'INSTANCED'
					// (Marking the first mesh per entity is enough)
					sortItem -= numMeshes;
					renderQueue.queue[sortItem->first].instanceSetIndex = pragma::rendering::RenderQueueItem::INSTANCED;
				}
				
				auto startSkipIndex = (sortItem -renderQueue.sortedItemIndices.data()) /sizeof(decltype(renderQueue.sortedItemIndices.front()));
				sortItem -= numMeshes;
				renderQueue.instanceSets.push_back({});
				renderQueue.queue[sortItem->first].instanceSetIndex = renderQueue.instanceSets.size() -1;
				auto &instanceSet = renderQueue.instanceSets.back();
				instanceSet.instanceCount = numInstances;
				instanceSet.instanceBuffer = instanceBuf;
				instanceSet.meshCount = numMeshes;
				instanceSet.startSkipIndex = startSkipIndex;

				instantiableEntityList = {};
				instantiableEntityList.push_back(newItem); // Restore the last item
			}
		}
		
		prevEntityHash = curEntityHash;
		curEntityHash = 0;
		
		if(renderC && sortItem->second.instantiable)
			instantiableEntityList.push_back(entIdx);
	};
	auto &sortedItemIndices = renderQueue.sortedItemIndices;
	auto fCalcNextEntityHash = [&sortedItemIndices,&renderQueue](uint32_t &inOutStartIndex,uint32_t &outNumMeshes) -> util::Hash {
		if(inOutStartIndex >= sortedItemIndices.size())
			return 0;
		util::Hash hash = 0;
		auto entity = renderQueue.queue[sortedItemIndices[inOutStartIndex].first].entity;
		uint32_t numMeshes = 0;
		while(inOutStartIndex < sortedItemIndices.size())
		{
			auto &sortKey = sortedItemIndices[inOutStartIndex];
			auto &item = renderQueue.queue[sortKey.first];
			if(item.entity != entity)
				break;
			++numMeshes;
			hash = util::hash_combine<uint64_t>(hash,*reinterpret_cast<uint64_t*>(&sortKey.second));
		}
		return hash;
	};

	auto fProcessInstantiable = [&renderQueue,instanceThreshold](uint32_t index,uint32_t numInstantiableEntities,uint32_t numMeshes) {
		if(numInstantiableEntities < instanceThreshold)
			return;
		auto startIndex = index -(numInstantiableEntities *numMeshes);

		std::vector<pragma::RenderBufferIndex> renderBufferIndices {};
		renderBufferIndices.reserve(instantiableEntityList.size());
		for(auto &entIdx : instantiableEntityList)
		{
			auto renderBufferIndex = static_cast<CBaseEntity*>(c_game->GetEntityByLocalIndex(entIdx))->GetRenderComponent()->GetRenderBufferIndex();
			renderBufferIndices.push_back(*renderBufferIndex);
		}
		
		auto &instanceIndexBuffer = pragma::CSceneComponent::GetEntityInstanceIndexBuffer();
		auto instanceBuf = instanceIndexBuffer->AddInstanceList(renderQueue,std::move(renderBufferIndices),util::hash_combine<uint64_t>(prevEntityHash,numInstantiableEntities));

		auto &instanceSet = renderQueue.instanceSets.back();
		instanceSet.instanceCount = numInstantiableEntities;
		instanceSet.instanceBuffer = instanceBuf;
		instanceSet.meshCount = numMeshes;
		instanceSet.startSkipIndex = startSkipIndex;
	};

	uint32_t prevNumMeshes = 0;
	uint32_t startIdx = 0;
	auto prevHash = fCalcNextEntityHash(startIdx,prevNumMeshes);
	uint32_t numInstantiableEntities = 1;
	uint32_t numMeshes = 0;
	while(startIdx < sortedItemIndices.size())
	{
		auto hash = fCalcNextEntityHash(startIdx,numMeshes);
		if(hash != prevHash) // New entity is different; no instantiation possible
		{
			// Process the instantiation list for everything before the current entity
			fProcessInstantiable(startIdx -numMeshes,numInstantiableEntities,prevNumMeshes);
			numInstantiableEntities = 1;
			continue;
		}
		++numInstantiableEntities;
		// Add to instance list
		// TODO
	}
	fProcessInstantiable(startIdx -numMeshes,numInstantiableEntities,prevNumMeshes);


	if(sortedItemIndices.empty() == false)
	{
		uint32_t curEntityMeshes = 0;
		auto prevEntityIndex = renderQueue.queue[sortedItemIndices.front().first].entity;
		for(auto i=decltype(sortedItemIndices.size()){0u};i<sortedItemIndices.size();++i)
		{
			auto &sortKey = sortedItemIndices.at(i);
			auto &item = renderQueue.queue[sortKey.first];
			if(item.entity != prevEntityIndex)
			{
				instantiableEntityList.push_back(entIdx);
				fUpdateEntityInstanceLists(renderQueue,item.entity,curEntityMeshes,&sortKey);

				prevEntityIndex = item.entity;
				curEntityMeshes = 0;
			}
			++curEntityMeshes;

			static_assert(sizeof(decltype(sortKey.second)) == sizeof(uint64_t));
			curEntityHash = util::hash_combine<uint64_t>(curEntityHash,*reinterpret_cast<uint64_t*>(&sortKey.second));
		}
		curEntityHash = 0;
		instantiableEntityList.push_back(std::numeric_limits<EntityIndex>::max());
		fUpdateEntityInstanceLists(renderQueue,instantiableEntityList.back(),curEntityMeshes,(&sortedItemIndices.back()) +1);
	}
#endif
}

static auto cvDrawWorld = GetClientConVar("render_draw_world");
static std::atomic<uint32_t> g_activeRenderQueueThreads = 0;
uint32_t SceneRenderDesc::GetActiveRenderQueueThreadCount() {return g_activeRenderQueueThreads;}
bool SceneRenderDesc::AssertRenderQueueThreadInactive()
{
	if(SceneRenderDesc::GetActiveRenderQueueThreadCount() == 0)
		return true;
	std::string msg = "ERROR: Game scene was changed during rendering, this is not allowed!";
	Con::crit<<msg<<Con::endl;
	throw std::logic_error{msg};
	return false;
}
static auto cvLockRenderQueues = GetClientConVar("debug_render_lock_render_queues");
void SceneRenderDesc::BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo)
{
	if(cvLockRenderQueues->GetBool())
		return;
	m_worldRenderQueuesReady = false;

	for(auto &renderQueue : m_renderQueues)
		renderQueue->Clear();
	m_worldRenderQueues.clear();

	auto &hCam = m_scene.GetActiveCamera();
	if(hCam.expired())
		return;

	for(auto &renderQueue : m_renderQueues)
		renderQueue->Lock();

	auto &cam = *hCam;
	// c_game->StartProfilingStage(CGame::CPUProfilingPhase::BuildRenderQueue);
	auto &posCam = g_debugFreezeCamData.has_value() ? g_debugFreezeCamData->pos : cam.GetEntity().GetPosition();

	c_game->GetRenderQueueBuilder().Append([this,&cam,posCam,&drawSceneInfo]() {
		++g_activeRenderQueueThreads;
		auto *stats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->renderQueueBuilderStats : nullptr;
		std::chrono::steady_clock::time_point tStart;
		if(stats)
		{
			auto &queueWorkerManager = c_game->GetRenderQueueWorkerManager();
			auto numWorkers = queueWorkerManager.GetWorkerCount();
			stats->workerStats.resize(numWorkers);
			for(auto i=decltype(numWorkers){0u};i<numWorkers;++i)
				queueWorkerManager.GetWorker(i).SetStats(&stats->workerStats[i]);
			tStart = std::chrono::steady_clock::now();
		}

		pragma::CSceneComponent::GetEntityInstanceIndexBuffer()->UpdateAndClearUnusedBuffers();

		auto &frustumPlanes = g_debugFreezeCamData.has_value() ? g_debugFreezeCamData->frustumPlanes : cam.GetFrustumPlanes();
		auto fShouldCull = [&frustumPlanes](const Vector3 &min,const Vector3 &max) -> bool {return SceneRenderDesc::ShouldCull(min,max,frustumPlanes);};
		auto vp = cam.GetProjectionMatrix() *cam.GetViewMatrix();

		std::vector<util::BSPTree::Node*> bspLeafNodes;
		std::vector<util::BSPTree*> bspTrees;
		// Note: World geometry is handled differently than other entities. World entities have their
		// own pre-built render queues, which we only have to iterate for maximum efficiency. Whether or not a world mesh is culled from the
		// camera frustum is stored in 'm_worldMeshVisibility', which is simply a boolean array so we don't have to copy any
		// data between render queues. (The data in 'm_worldMeshVisibility' is only valid for this render pass.)
		// Translucent world meshes still need to be sorted with other entity meshes, so they are just copied over to the
		// main render queue.

		std::chrono::steady_clock::time_point t;
		if(stats)
			t = std::chrono::steady_clock::now();

		EntityIterator entItWorld {*c_game};
		entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
		bspLeafNodes.reserve(entItWorld.GetCount());
		bspTrees.reserve(entItWorld.GetCount());
		m_worldMeshVisibility.reserve(entItWorld.GetCount());
		for(auto *entWorld : entItWorld)
		{
			if(ShouldConsiderEntity(*static_cast<CBaseEntity*>(entWorld),m_scene,drawSceneInfo.renderFlags) == false)
				continue;
			auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
			auto &bspTree = worldC->GetBSPTree();
			auto *node = bspTree ? bspTree->FindLeafNode(posCam) : nullptr;
			if(node == nullptr)
				continue;
			bspLeafNodes.push_back(node);
			bspTrees.push_back(bspTree.get());

			if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Static) == false)
				continue;

			auto *renderC = static_cast<CBaseEntity&>(worldC->GetEntity()).GetRenderComponent();
			renderC->UpdateRenderDataMT(drawSceneInfo.commandBuffer,m_scene,cam,vp);
			auto *renderQueue = worldC->GetClusterRenderQueue(node->cluster);
			if(renderQueue)
			{
				auto idx = m_worldRenderQueues.size();
				if(idx >= m_worldMeshVisibility.size())
					m_worldMeshVisibility.push_back({});
				auto &worldMeshVisibility = m_worldMeshVisibility.at(idx);

				m_worldRenderQueues.push_back(renderQueue->shared_from_this());
				auto &pos = entWorld->GetPosition();
				auto &renderMeshes = renderC->GetRenderMeshes();
				worldMeshVisibility.resize(renderMeshes.size());
				for(auto i=decltype(renderQueue->queue.size()){0u};i<renderQueue->queue.size();++i)
				{
					auto &item = renderQueue->queue.at(i);
					if(item.mesh >= renderMeshes.size())
						continue;
					worldMeshVisibility.at(item.mesh) = !ShouldCull(*renderC,item.mesh,fShouldCull);
				}
			}

			if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Translucent))
			{
				// Translucent meshes will have to be sorted dynamically with all other non-world translucent objects,
				// so we'll copy the information to the dynamic queue
				auto *renderQueueTranslucentSrc = worldC->GetClusterRenderQueue(node->cluster,true /* translucent */);
				auto *renderQueueTranslucentDst = renderQueueTranslucentSrc ? GetRenderQueue(static_cast<CBaseEntity*>(entWorld)->GetRenderComponent()->GetRenderMode(),true) : nullptr;
				if(renderQueueTranslucentDst == nullptr || renderQueueTranslucentSrc->queue.empty())
					continue;
				renderQueueTranslucentDst->queue.reserve(renderQueueTranslucentDst->queue.size() +renderQueueTranslucentSrc->queue.size());
				renderQueueTranslucentDst->sortedItemIndices.reserve(renderQueueTranslucentDst->queue.size());
				auto &pose = entWorld->GetPose();
				for(auto i=decltype(renderQueueTranslucentSrc->queue.size()){0u};i<renderQueueTranslucentSrc->queue.size();++i)
				{
					auto &item = renderQueueTranslucentSrc->queue.at(i);
					if(ShouldCull(*renderC,item.mesh,fShouldCull))
						continue;
					renderQueueTranslucentDst->queue.push_back(item);
					renderQueueTranslucentDst->sortedItemIndices.push_back(renderQueueTranslucentSrc->sortedItemIndices.at(i));
					renderQueueTranslucentDst->sortedItemIndices.back().first = renderQueueTranslucentDst->queue.size() -1;

					auto &renderMeshes = renderC->GetRenderMeshes();
					if(item.mesh >= renderMeshes.size())
						continue;
					auto &pos = pose *renderMeshes[item.mesh]->GetCenter();
					renderQueueTranslucentDst->queue.back().sortingKey.SetDistance(pos,cam);
				}
			}
		}
		m_worldMeshVisibility.resize(m_worldRenderQueues.size());
		m_worldRenderQueuesReady = true;

		if(stats)
			(*stats)->AddTime(RenderQueueBuilderStats::Timer::WorldQueueUpdate,std::chrono::steady_clock::now() -t);

		if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Dynamic))
		{
			// Some entities are exempt from occlusion culling altogether, we'll handle them here
			for(auto *pRenderComponent : pragma::CRenderComponent::GetEntitiesExemptFromOcclusionCulling())
			{
				if(ShouldConsiderEntity(static_cast<CBaseEntity&>(pRenderComponent->GetEntity()),m_scene,drawSceneInfo.renderFlags) == false)
					continue;
				AddRenderMeshesToRenderQueue(drawSceneInfo,*pRenderComponent,m_scene,cam,vp,nullptr);
			}

			if(stats)
				t = std::chrono::steady_clock::now();

			// Now we just need the remaining entities, for which we'll use the scene octree
			auto *culler = m_scene.FindOcclusionCuller();
			if(culler)
			{
				auto &dynOctree = culler->GetOcclusionOctree();
				CollectRenderMeshesFromOctree(drawSceneInfo,dynOctree,m_scene,cam,vp,drawSceneInfo.renderFlags,frustumPlanes,&bspTrees,&bspLeafNodes);
			}

			if(stats)
				(*stats)->AddTime(RenderQueueBuilderStats::Timer::OctreeProcessing,std::chrono::steady_clock::now() -t);
		}

		if(stats)
			t = std::chrono::steady_clock::now();
		c_game->GetRenderQueueWorkerManager().WaitForCompletion();
		if(stats)
			(*stats)->AddTime(RenderQueueBuilderStats::Timer::WorkerWait,std::chrono::steady_clock::now() -t);

		// All render queues (aside from world render queues) need to be sorted
		for(auto &renderQueue : m_renderQueues)
		{
			if(stats)
				t = std::chrono::steady_clock::now();
			renderQueue->Sort();
			if(stats)
				(*stats)->AddTime(RenderQueueBuilderStats::Timer::QueueSort,std::chrono::steady_clock::now() -t);
			
			if(stats)
				t = std::chrono::steady_clock::now();
			BuildRenderQueueInstanceLists(*renderQueue);
			if(stats)
				(*stats)->AddTime(RenderQueueBuilderStats::Timer::QueueInstancing,std::chrono::steady_clock::now() -t);
			renderQueue->Unlock();
		}
		// c_game->StopProfilingStage(CGame::CPUProfilingPhase::BuildRenderQueue);
		if(stats)
		{
			(*stats)->AddTime(RenderQueueBuilderStats::Timer::TotalExecution,std::chrono::steady_clock::now() -tStart);
			auto &queueWorkerManager = c_game->GetRenderQueueWorkerManager();
			auto numWorkers = queueWorkerManager.GetWorkerCount();
			for(auto i=decltype(numWorkers){0u};i<numWorkers;++i)
				queueWorkerManager.GetWorker(i).SetStats(nullptr);
		}
		--g_activeRenderQueueThreads;
	});
}
#pragma optimize("",on)
