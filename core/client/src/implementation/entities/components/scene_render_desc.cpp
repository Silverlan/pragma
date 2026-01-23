// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.scene;
import :client_state;
import :engine;
import :game;
import :rendering.render_queue_instancer;

#undef AddJob

SceneRenderDesc::SceneRenderDesc(pragma::CSceneComponent &scene) : m_scene {scene}
{
	auto name = scene.GetEntity().GetName();
	std::string prefix = "scene_";
	if(!name.empty())
		prefix += name + '_';
	for(uint32_t i = 0; auto &renderQueue : m_renderQueues)
		renderQueue = pragma::rendering::RenderQueue::Create(prefix + std::string {magic_enum::enum_name(static_cast<RenderQueueId>(i++))});
}
SceneRenderDesc::~SceneRenderDesc() {}

static auto cvDrawGlow = pragma::console::get_client_con_var("render_draw_glow");
static auto cvDrawTranslucent = pragma::console::get_client_con_var("render_draw_translucent");
static auto cvDrawSky = pragma::console::get_client_con_var("render_draw_sky");
static auto cvDrawWater = pragma::console::get_client_con_var("render_draw_water");
static auto cvDrawView = pragma::console::get_client_con_var("render_draw_view");
static pragma::rendering::RenderFlags render_mode_to_render_flag(pragma::rendering::SceneRenderPass renderMode)
{
	switch(renderMode) {
	case pragma::rendering::SceneRenderPass::World:
		return pragma::rendering::RenderFlags::World;
	case pragma::rendering::SceneRenderPass::View:
		return pragma::rendering::RenderFlags::View;
	case pragma::rendering::SceneRenderPass::Sky:
		return pragma::rendering::RenderFlags::Skybox;
	}
	static_assert(pragma::math::to_integral(pragma::rendering::SceneRenderPass::Count) == 4);
	return pragma::rendering::RenderFlags::None;
}

SceneRenderDesc::RenderQueueId SceneRenderDesc::GetRenderQueueId(pragma::rendering::SceneRenderPass renderMode, bool translucent) const
{
	switch(renderMode) {
	case pragma::rendering::SceneRenderPass::Sky:
		return !translucent ? RenderQueueId::Skybox : RenderQueueId::SkyboxTranslucent;
	case pragma::rendering::SceneRenderPass::View:
		return !translucent ? RenderQueueId::View : RenderQueueId::ViewTranslucent;
	// case pragma::rendering::SceneRenderPass::Water:
	// 	return !translucent ? RenderQueueId::Water : RenderQueueId::Invalid;
	case pragma::rendering::SceneRenderPass::World:
		return !translucent ? RenderQueueId::World : RenderQueueId::WorldTranslucent;
	}
	static_assert(pragma::math::to_integral(RenderQueueId::Count) == 8u);
	return RenderQueueId::Invalid;
}
pragma::rendering::RenderQueue *SceneRenderDesc::GetRenderQueue(pragma::rendering::SceneRenderPass renderMode, bool translucent)
{
	auto renderQueueId = GetRenderQueueId(renderMode, translucent);
	return (renderQueueId != RenderQueueId::Invalid) ? m_renderQueues.at(pragma::math::to_integral(renderQueueId)).get() : nullptr;
}
const pragma::rendering::RenderQueue *SceneRenderDesc::GetRenderQueue(pragma::rendering::SceneRenderPass renderMode, bool translucent) const { return const_cast<SceneRenderDesc *>(this)->GetRenderQueue(renderMode, translucent); }
const std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> &SceneRenderDesc::GetWorldRenderQueues() const { return m_worldRenderQueues; }

void SceneRenderDesc::AddRenderMeshesToRenderQueue(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, pragma::CRenderComponent &renderC, const std::function<pragma::rendering::RenderQueue *(pragma::rendering::SceneRenderPass, bool)> &getRenderQueue,
  const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam, const Mat4 &vp, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull, int32_t lodBias,
  const std::function<void(pragma::rendering::RenderQueue &, const pragma::rendering::RenderQueueItem &)> &fOptInsertItemToQueue, pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags)
{
	auto *mdlC = renderC.GetModelComponent();
	auto lod = pragma::math::max(static_cast<int32_t>(mdlC->GetLOD()) + lodBias, 0);
	auto &renderMeshes = renderC.GetRenderMeshes();
	auto &renderBufferData = renderC.GetRenderBufferData();
	auto &lodGroup = renderC.GetLodRenderMeshGroup(lod);
	renderC.UpdateRenderDataMT(scene, cam, vp);
	auto renderMode = renderC.GetSceneRenderPass();
	auto baseShaderSpecializationFlags = mdlC->GetBaseShaderSpecializationFlags() | baseSpecializationFlags;
	auto isBaseTranslucent = pragma::math::is_flag_set(baseShaderSpecializationFlags, pragma::GameShaderSpecializationConstantFlag::EnableTranslucencyBit);
	auto &context = pragma::get_cengine()->GetRenderContext();
	auto renderTranslucent = pragma::math::is_flag_set(renderFlags, pragma::rendering::RenderFlags::Translucent);
	for(auto meshIdx = lodGroup.first; meshIdx < lodGroup.first + lodGroup.second; ++meshIdx) {
		if(fShouldCull && ShouldCull(renderC, meshIdx, fShouldCull))
			continue;
		auto &renderMesh = renderMeshes[meshIdx];
		auto *mat = static_cast<pragma::material::CMaterial *>(renderBufferData[meshIdx].material.get());
		if(mat == nullptr) {
			if(pragma::rendering::VERBOSE_RENDER_OUTPUT_ENABLED) {
				Con::CWAR << "[RenderQueue] WARNING: Entity" << mdlC->GetEntity() << " has invalid render material!" << Con::endl;
			}
			continue;
		}
		auto *shader = static_cast<pragma::ShaderGameWorldLightingPass *>(mat->GetUserData2());
		if(shader == nullptr) {
			// TODO: Shaders are initialized lazily and calling GetPrimaryShader may invoke the load process.
			// This is illegal here, since this function may be called from a thread other than the main thread!
			shader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(mat->GetPrimaryShader());
			mat->SetUserData2(shader); // TODO: This is technically *not* thread safe and could be called from multiple threads!
		}
		if(optRasterizationRenderer)
			shader = optRasterizationRenderer->GetShaderOverride(shader);
		if(shader == nullptr) {
			if(pragma::rendering::VERBOSE_RENDER_OUTPUT_ENABLED) {
				Con::CWAR << "[RenderQueue] WARNING: Entity" << mdlC->GetEntity() << " has invalid render material shader!" << Con::endl;
			}
			continue;
		}
		auto specializationFlags = baseShaderSpecializationFlags | renderBufferData[meshIdx].pipelineSpecializationFlags | shader->GetBaseSpecializationFlags();
		auto pipelineIdx = shader->FindPipelineIndex(pragma::rendering::PassType::Generic, // The translation to the actual pass type will happen in the render processor
		  renderC.GetShaderPipelineSpecialization(), specializationFlags);
		prosper::PipelineID pipelineId;
		if(pipelineIdx.has_value() == false || shader->GetPipelineId(pipelineId, *pipelineIdx) == false || pipelineId == std::numeric_limits<decltype(pipelineId)>::max()) {
			if(pragma::rendering::VERBOSE_RENDER_OUTPUT_ENABLED) {
				Con::CWAR << "[RenderQueue] WARNING: Entity" << mdlC->GetEntity() << " has specialization flags referring to invalid shader pipeline!" << Con::endl;
			}
			continue;
		}
		// shader = rasterizationRenderer->GetShaderOverride(shader);
		// if(shader == nullptr)
		// 	continue;
		auto translucent = shader->IsTranslucentPipeline(*pipelineIdx) || isBaseTranslucent;
		if(renderTranslucent == false && translucent)
			continue;
		auto *renderQueue = getRenderQueue(renderMode, translucent);
		if(renderQueue == nullptr)
			continue;
		auto matIdx = mat->GetIndex();
		if(matIdx == std::numeric_limits<decltype(matIdx)>::max()) {
			if(pragma::rendering::VERBOSE_RENDER_OUTPUT_ENABLED) {
				Con::CWAR << "[RenderQueue] WARNING: Entity" << mdlC->GetEntity() << " has unindexed material '" << mat->GetName() << "'!" << Con::endl;
			}
			continue;
		}

		pragma::rendering::RenderQueueItem::TranslucencyPassInfo translucenyPassInfo {cam};
		if(translucent) {
			auto &distanceBiasSqr = renderC.GetTranslucencyPassDistanceOverrideSqr();
			if(distanceBiasSqr)
				translucenyPassInfo.distanceOverrideSqr = distanceBiasSqr;
		}

		pragma::rendering::RenderQueueItem item {static_cast<pragma::ecs::CBaseEntity &>(renderC.GetEntity()), meshIdx, *mat, pipelineId, translucent ? &translucenyPassInfo : nullptr};
		if(fOptInsertItemToQueue)
			fOptInsertItemToQueue(*renderQueue, item);
		else
			renderQueue->Add(item);
	}
}
void SceneRenderDesc::AddRenderMeshesToRenderQueue(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, pragma::CRenderComponent &renderC, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam, const Mat4 &vp,
  const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull, pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags)
{
	AddRenderMeshesToRenderQueue(optRasterizationRenderer, renderFlags, renderC, [this](pragma::rendering::SceneRenderPass renderMode, bool translucent) { return GetRenderQueue(renderMode, translucent); }, scene, cam, vp, fShouldCull, 0, nullptr, baseSpecializationFlags);
}

bool SceneRenderDesc::ShouldCull(pragma::ecs::CBaseEntity &ent, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull)
{
	auto *renderC = ent.GetRenderComponent();
	return !renderC || ShouldCull(*renderC, fShouldCull);
}
bool SceneRenderDesc::ShouldCull(pragma::CRenderComponent &renderC, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull)
{
	auto &aabb = renderC.GetAbsoluteRenderBounds();
	return fShouldCull(aabb.min, aabb.max);
}
bool SceneRenderDesc::ShouldCull(pragma::CRenderComponent &renderC, pragma::rendering::RenderMeshIndex meshIdx, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull)
{
	auto &renderMeshes = renderC.GetRenderMeshes();
	if(meshIdx >= renderMeshes.size())
		return false;
	auto &renderMesh = renderMeshes[meshIdx];
	Vector3 min, max;
	renderMesh->GetBounds(min, max);
	auto &scale = renderC.GetEntity().GetScale();
	min *= scale;
	max *= scale;
	auto &pos = renderC.GetEntity().GetPosition();
	min += pos;
	max += pos;
	return fShouldCull(min, max);
}
bool SceneRenderDesc::ShouldCull(const Vector3 &min, const Vector3 &max, const std::vector<pragma::math::Plane> &frustumPlanes) { return pragma::math::intersection::aabb_in_plane_mesh(min, max, frustumPlanes.begin(), frustumPlanes.end()) == pragma::math::intersection::Intersect::Outside; }

static auto cvEntitiesPerJob = pragma::console::get_client_con_var("render_queue_entities_per_worker_job");
void SceneRenderDesc::CollectRenderMeshesFromOctree(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, bool enableClipping, const OcclusionOctree<pragma::ecs::CBaseEntity *> &tree, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam,
  const Mat4 &vp, pragma::rendering::RenderMask renderMask, const std::function<pragma::rendering::RenderQueue *(pragma::rendering::SceneRenderPass, bool)> &getRenderQueue, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull,
  const std::vector<pragma::util::BSPTree *> *bspTrees, const std::vector<pragma::util::BSPTree::Node *> *bspLeafNodes, int32_t lodBias, const std::function<bool(pragma::ecs::CBaseEntity &, const pragma::CSceneComponent &, pragma::rendering::RenderFlags)> &shouldConsiderEntity,
  pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags)
{
	auto numEntitiesPerWorkerJob = pragma::math::max(cvEntitiesPerJob->GetInt(), 1);
	std::function<void(const OcclusionOctree<pragma::ecs::CBaseEntity *>::Node &node)> iterateTree = nullptr;
	iterateTree = [&iterateTree, &shouldConsiderEntity, &scene, &cam, renderFlags, fShouldCull, optRasterizationRenderer, renderMask, &getRenderQueue, &vp, bspLeafNodes, bspTrees, lodBias, numEntitiesPerWorkerJob, baseSpecializationFlags](const OcclusionOctree<pragma::ecs::CBaseEntity *>::Node &node) {
		auto &nodeBounds = node.GetWorldBounds();
		if(fShouldCull && fShouldCull(nodeBounds.first, nodeBounds.second))
			return;
		if(bspLeafNodes && bspLeafNodes->empty() == false) {
			auto hasIntersection = false;
			for(auto i = decltype(bspLeafNodes->size()) {0u}; i < bspLeafNodes->size(); ++i) {
				auto *node = (*bspLeafNodes)[i];
				if(pragma::math::intersection::aabb_aabb(nodeBounds.first, nodeBounds.second, node->minVisible, node->maxVisible) == pragma::math::intersection::Intersect::Outside)
					continue;
				if((*bspTrees)[i]->IsAabbVisibleInCluster(nodeBounds.first, nodeBounds.second, node->cluster) == false)
					continue;
				hasIntersection = true;
				break;
			}
			if(hasIntersection == false)
				return;
		}
		auto &objs = node.GetObjects();
		auto numObjects = objs.size();
		auto numBatches = (numObjects / numEntitiesPerWorkerJob) + (((numObjects % numEntitiesPerWorkerJob) > 0) ? 1 : 0);
		for(auto i = decltype(numBatches) {0u}; i < numBatches; ++i) {
			auto iStart = i * numEntitiesPerWorkerJob;
			auto iEnd = pragma::math::min(static_cast<size_t>(iStart + numEntitiesPerWorkerJob), numObjects);
			pragma::get_cgame()->GetRenderQueueWorkerManager().AddJob([iStart, iEnd, shouldConsiderEntity, renderMask, optRasterizationRenderer, &objs, renderFlags, getRenderQueue, &scene, &cam, vp, fShouldCull, lodBias, baseSpecializationFlags]() {
				// Note: We don't add individual items directly to the render queue, because that would invoke
				// a mutex lock which can stall all of the worker threads.
				// Instead we'll collect the entire batch of items, then add all of them to the render queue at once.
				std::unordered_map<pragma::rendering::RenderQueue *, std::vector<pragma::rendering::RenderQueueItem>> items;
				for(auto i = iStart; i < iEnd; ++i) {
					auto *ent = objs[i];
					assert(ent);
					if(ent == nullptr) {
						// This should NEVER occur, but seems to anyway in some rare cases
						Con::CERR << "NULL Entity in dynamic scene occlusion octree! Ignoring..." << Con::endl;
						continue;
					}

					if(ent->IsWorld()) {
						auto worldC = ent->GetComponent<pragma::CWorldComponent>();
						if(worldC.valid() && worldC->GetBSPTree())
							continue; // World entities with BSP trees are handled separately
					}
					auto *renderC = static_cast<pragma::ecs::CBaseEntity *>(ent)->GetRenderComponent();
					if(!renderC || renderC->IsExemptFromOcclusionCulling() || ShouldConsiderEntity(*static_cast<pragma::ecs::CBaseEntity *>(ent), scene, renderFlags, renderMask) == false
					  || (shouldConsiderEntity && shouldConsiderEntity(*static_cast<pragma::ecs::CBaseEntity *>(ent), scene, renderFlags) == false) //||
					                                                                                                                   //(camClusterIdx.has_value() && renderC->IsVisibleInCluster(*camClusterIdx) == false)
					)
						continue;
					if(fShouldCull && ShouldCull(*renderC, fShouldCull))
						continue;
					AddRenderMeshesToRenderQueue(
					  optRasterizationRenderer, renderFlags, *renderC, getRenderQueue, scene, cam, vp, fShouldCull, lodBias,
					  [&items](pragma::rendering::RenderQueue &renderQueue, const pragma::rendering::RenderQueueItem &item) {
						  auto &v = items[&renderQueue];
						  if(v.size() == v.capacity())
							  v.reserve(v.size() * 1.1f + 50);
						  v.push_back(item);
					  },
					  baseSpecializationFlags);
				}
				for(auto &pair : items)
					pair.first->Add(pair.second);
			});
		}
		auto *children = node.GetChildren();
		if(children == nullptr)
			return;
		for(auto &c : *children) {
			if(!c)
				continue;
			iterateTree(static_cast<OcclusionOctree<pragma::ecs::CBaseEntity *>::Node &>(*c));
		}
	};
	iterateTree(tree.GetRootNode());
}
void SceneRenderDesc::CollectRenderMeshesFromOctree(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, bool enableClipping, const OcclusionOctree<pragma::ecs::CBaseEntity *> &tree, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam,
  const Mat4 &vp, pragma::rendering::RenderMask renderMask, const std::vector<pragma::math::Plane> &frustumPlanes, const std::vector<pragma::util::BSPTree *> *bspTrees, const std::vector<pragma::util::BSPTree::Node *> *bspLeafNodes)
{
	CollectRenderMeshesFromOctree(
	  optRasterizationRenderer, renderFlags, enableClipping, tree, scene, cam, vp, renderMask, [this](pragma::rendering::SceneRenderPass renderMode, bool translucent) { return GetRenderQueue(renderMode, translucent); },
	  [frustumPlanes](const Vector3 &min, const Vector3 &max) -> bool { return pragma::math::intersection::aabb_in_plane_mesh(min, max, frustumPlanes.begin(), frustumPlanes.end()) == pragma::math::intersection::Intersect::Outside; }, bspTrees, bspLeafNodes, 0, nullptr);
}

bool SceneRenderDesc::ShouldConsiderEntity(pragma::ecs::CBaseEntity &ent, const pragma::CSceneComponent &scene, pragma::rendering::RenderFlags renderFlags, pragma::rendering::RenderMask renderMask)
{
	if(ent.IsInScene(scene) == false || !ent.GetRenderComponent())
		return false;
	auto *renderC = ent.GetRenderComponent();
	auto renderMode = renderC->GetSceneRenderPass();
	auto renderGroups = renderC->GetRenderGroups();
	return pragma::math::is_flag_set(renderFlags, render_mode_to_render_flag(renderMode)) && (renderGroups & renderMask) == renderGroups && ent.GetModel() != nullptr && renderC->ShouldDraw();
}

struct DebugFreezeCamData {
	Vector3 pos;
	std::vector<pragma::math::Plane> frustumPlanes;
};
static std::optional<DebugFreezeCamData> g_debugFreezeCamData = {};
static void cmd_debug_occlusion_culling_freeze_camera(pragma::NetworkState *, const pragma::console::ConVar &, bool, bool val)
{
	g_debugFreezeCamData = {};
	if(val == false)
		return;
	auto *scene = pragma::get_cgame()->GetRenderScene<pragma::CSceneComponent>();
	if(scene == nullptr)
		return;
	auto &cam = scene->GetActiveCamera();
	if(cam.expired())
		return;
	g_debugFreezeCamData = DebugFreezeCamData {};
	g_debugFreezeCamData->pos = cam->GetEntity().GetPosition();
	g_debugFreezeCamData->frustumPlanes = cam->GetFrustumPlanes();
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("debug_occlusion_culling_freeze_camera", &cmd_debug_occlusion_culling_freeze_camera);
}

bool SceneRenderDesc::IsWorldMeshVisible(uint32_t worldRenderQueueIndex, pragma::rendering::RenderMeshIndex meshIdx) const
{
	if(worldRenderQueueIndex >= m_worldMeshVisibility.size())
		return false;
	auto &worldMeshVisibility = m_worldMeshVisibility.at(worldRenderQueueIndex);
	return (meshIdx < worldMeshVisibility.size()) ? worldMeshVisibility.at(meshIdx) : false;
}

void SceneRenderDesc::WaitForWorldRenderQueues() const
{
	while(m_worldRenderQueuesReady == false)
		;
}

static auto cvInstancingEnabled = pragma::console::get_client_con_var("render_instancing_enabled");
void SceneRenderDesc::BuildRenderQueueInstanceLists(pragma::rendering::RenderQueue &renderQueue)
{
	renderQueue.instanceSets.clear();
	if(cvInstancingEnabled->GetBool() == false)
		return;
	pragma::rendering::RenderQueueInstancer instancer {renderQueue};
	instancer.Process();

#if 0
	pragma::util::Hash prevEntityHash = 0;
	pragma::util::Hash curEntityHash = 0;
	uint32_t numHashMatches = 0;
	auto instanceThreshold = pragma::math::max(cvInstancingThreshold->GetInt(),2);
	std::vector<EntityIndex> instantiableEntityList;
	auto fUpdateEntityInstanceLists = [&instantiableEntityList,&curEntityHash,&prevEntityHash,instanceThreshold](
		pragma::rendering::RenderQueue &renderQueue,EntityIndex entIdx,uint32_t numMeshes,pragma::rendering::RenderQueueItemSortPair *sortItem
	) {
		// New entity
		auto *ent = static_cast<pragma::ecs::CBaseEntity*>(pragma::get_cgame()->GetEntityByLocalIndex(entIdx));
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
					auto renderBufferIndex = static_cast<pragma::ecs::CBaseEntity*>(pragma::get_cgame()->GetEntityByLocalIndex(entIdx))->GetRenderComponent()->GetRenderBufferIndex();
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
	auto fCalcNextEntityHash = [&sortedItemIndices,&renderQueue](uint32_t &inOutStartIndex,uint32_t &outNumMeshes) -> pragma::util::Hash {
		if(inOutStartIndex >= sortedItemIndices.size())
			return 0;
		pragma::util::Hash hash = 0;
		auto entity = renderQueue.queue[sortedItemIndices[inOutStartIndex].first].entity;
		uint32_t numMeshes = 0;
		while(inOutStartIndex < sortedItemIndices.size())
		{
			auto &sortKey = sortedItemIndices[inOutStartIndex];
			auto &item = renderQueue.queue[sortKey.first];
			if(item.entity != entity)
				break;
			++numMeshes;
			hash = pragma::util::hash_combine<uint64_t>(hash,*reinterpret_cast<uint64_t*>(&sortKey.second));
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
			auto renderBufferIndex = static_cast<pragma::ecs::CBaseEntity*>(pragma::get_cgame()->GetEntityByLocalIndex(entIdx))->GetRenderComponent()->GetRenderBufferIndex();
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
			curEntityHash = pragma::util::hash_combine<uint64_t>(curEntityHash,*reinterpret_cast<uint64_t*>(&sortKey.second));
		}
		curEntityHash = 0;
		instantiableEntityList.push_back(std::numeric_limits<EntityIndex>::max());
		fUpdateEntityInstanceLists(renderQueue,instantiableEntityList.back(),curEntityMeshes,(&sortedItemIndices.back()) +1);
	}
#endif
}

static auto cvDrawWorld = pragma::console::get_client_con_var("render_draw_world");
static std::atomic<uint32_t> g_activeRenderQueueThreads = 0;
uint32_t SceneRenderDesc::GetActiveRenderQueueThreadCount() { return g_activeRenderQueueThreads; }
bool SceneRenderDesc::AssertRenderQueueThreadInactive()
{
	if(GetActiveRenderQueueThreadCount() == 0)
		return true;
	std::string msg = "Game scene was changed during rendering, this is not allowed!";
	Con::CRIT << msg << Con::endl;
	throw std::logic_error {msg};
	return false;
}
static auto cvLockRenderQueues = pragma::console::get_client_con_var("debug_render_lock_render_queues");
static auto cvFrustumCullingEnabled = pragma::console::get_client_con_var("cl_render_frustum_culling_enabled");
// Tag: render-queues
void SceneRenderDesc::BuildRenderQueues(const pragma::rendering::DrawSceneInfo &drawSceneInfo, const std::function<void()> &fBuildAdditionalQueues)
{
	if(cvLockRenderQueues->GetBool())
		return;
	m_worldRenderQueuesReady = false;

	for(auto &renderQueue : m_renderQueues)
		renderQueue->Clear();
	m_worldRenderQueues.clear();

	auto *renderer = drawSceneInfo.scene->GetRenderer<pragma::CRendererComponent>();
	auto hRasterizer = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	auto &hCam = m_scene.GetActiveCamera();
	if(hCam.expired() || hRasterizer.expired()) {
		m_worldRenderQueuesReady = true;
		return;
	}

	for(auto &renderQueue : m_renderQueues)
		renderQueue->Lock();

	auto &cam = *hCam;
	auto &rasterizer = *hRasterizer;
	// pragma::get_cgame()->StartProfilingStage(pragma::CGame::CPUProfilingPhase::BuildRenderQueue);
	auto &posCam = g_debugFreezeCamData.has_value() ? g_debugFreezeCamData->pos : drawSceneInfo.pvsOrigin.has_value() ? *drawSceneInfo.pvsOrigin : cam.GetEntity().GetPosition();

	auto renderMask = drawSceneInfo.GetRenderMask(*pragma::get_cgame());
	renderMask |= drawSceneInfo.scene->GetInclusionRenderMask();
	renderMask |= drawSceneInfo.scene->GetExclusionRenderMask();
	auto tStart = std::chrono::steady_clock::now();
	pragma::get_cgame()->GetRenderQueueBuilder().Append(
	  [this, &rasterizer, &cam, posCam, &drawSceneInfo, renderMask]() {
		  ++g_activeRenderQueueThreads;
		  auto *stats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->renderQueueBuilderStats : nullptr;
		  if(stats) {
			  auto &queueWorkerManager = pragma::get_cgame()->GetRenderQueueWorkerManager();
			  auto numWorkers = queueWorkerManager.GetWorkerCount();
			  stats->workerStats.resize(numWorkers);
			  for(auto i = decltype(numWorkers) {0u}; i < numWorkers; ++i)
				  queueWorkerManager.GetWorker(i).SetStats(&stats->workerStats[i]);
		  }

		  pragma::CSceneComponent::GetEntityInstanceIndexBuffer()->UpdateAndClearUnusedBuffers();

		  auto frustumCullingEnabled = cvFrustumCullingEnabled->GetBool();
		  static std::vector<pragma::math::Plane> frustumPlanesCube {};
		  if(frustumCullingEnabled == false && frustumPlanesCube.empty()) {
			  double d = pragma::cCameraComponent::DEFAULT_FAR_Z;
			  frustumPlanesCube = {pragma::math::Plane {-uvec::PRM_RIGHT, d}, pragma::math::Plane {uvec::PRM_RIGHT, d}, pragma::math::Plane {uvec::PRM_UP, d}, pragma::math::Plane {-uvec::PRM_UP, d}, pragma::math::Plane {-uvec::PRM_FORWARD, d}, pragma::math::Plane {uvec::PRM_FORWARD, d}};
		  }
		  std::vector<pragma::math::Plane> frustumPlanes;
		  if(!frustumCullingEnabled)
			  frustumPlanes = frustumPlanesCube;
		  else if(g_debugFreezeCamData.has_value())
			  frustumPlanes = g_debugFreezeCamData->frustumPlanes;
		  else
			  cam.GetFrustumPlanes(frustumPlanes);
		  if(drawSceneInfo.clipPlane.has_value())
			  frustumPlanes.push_back(*drawSceneInfo.clipPlane);
		  auto fShouldCull = [&frustumPlanes](const Vector3 &min, const Vector3 &max) -> bool { return ShouldCull(min, max, frustumPlanes); };
		  auto vp = cam.GetProjectionMatrix() * cam.GetViewMatrix();

		  std::vector<pragma::util::BSPTree::Node *> bspLeafNodes;
		  std::vector<pragma::util::BSPTree *> bspTrees;
		  // Note: World geometry is handled differently than other entities. World entities have their
		  // own pre-built render queues, which we only have to iterate for maximum efficiency. Whether or not a world mesh is culled from the
		  // camera frustum is stored in 'm_worldMeshVisibility', which is simply a boolean array so we don't have to copy any
		  // data between render queues. (The data in 'm_worldMeshVisibility' is only valid for this render pass.)
		  // Translucent world meshes still need to be sorted with other entity meshes, so they are just copied over to the
		  // main render queue.

		  std::chrono::steady_clock::time_point t;
		  if(stats)
			  t = std::chrono::steady_clock::now();

		  pragma::ecs::EntityIterator entItWorld {*pragma::get_cgame()};
		  entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
		  bspLeafNodes.reserve(entItWorld.GetCount());
		  bspTrees.reserve(entItWorld.GetCount());
		  m_worldMeshVisibility.reserve(entItWorld.GetCount());
		  for(auto *entWorld : entItWorld) {
			  if(ShouldConsiderEntity(*static_cast<pragma::ecs::CBaseEntity *>(entWorld), m_scene, drawSceneInfo.renderFlags, renderMask) == false)
				  continue;
			  auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
			  auto &bspTree = worldC->GetBSPTree();
			  auto *node = bspTree ? bspTree->FindLeafNode(posCam) : nullptr;
			  if(node == nullptr)
				  continue;
			  bspLeafNodes.push_back(node);
			  bspTrees.push_back(bspTree.get());

			  if(pragma::math::is_flag_set(drawSceneInfo.renderFlags, pragma::rendering::RenderFlags::Static) == false)
				  continue;

			  auto *renderC = static_cast<pragma::ecs::CBaseEntity &>(worldC->GetEntity()).GetRenderComponent();
			  renderC->UpdateRenderDataMT(m_scene, cam, vp);
			  auto *renderQueue = worldC->GetClusterRenderQueue(node->cluster);
			  if(renderQueue) {
				  auto idx = m_worldRenderQueues.size();
				  if(idx >= m_worldMeshVisibility.size())
					  m_worldMeshVisibility.push_back({});
				  auto &worldMeshVisibility = m_worldMeshVisibility.at(idx);

				  m_worldRenderQueues.push_back(renderQueue->shared_from_this());
				  auto &pos = entWorld->GetPosition();
				  auto &renderMeshes = renderC->GetRenderMeshes();
				  worldMeshVisibility.resize(renderMeshes.size());
				  for(auto i = decltype(renderQueue->queue.size()) {0u}; i < renderQueue->queue.size(); ++i) {
					  auto &item = renderQueue->queue.at(i);
					  if(item.mesh >= renderMeshes.size())
						  continue;
					  worldMeshVisibility.at(item.mesh) = !ShouldCull(*renderC, item.mesh, fShouldCull);
				  }
			  }

			  if(pragma::math::is_flag_set(drawSceneInfo.renderFlags, pragma::rendering::RenderFlags::Translucent)) {
				  // Translucent meshes will have to be sorted dynamically with all other non-world translucent objects,
				  // so we'll copy the information to the dynamic queue
				  auto *renderQueueTranslucentSrc = worldC->GetClusterRenderQueue(node->cluster, true /* translucent */);
				  auto *renderQueueTranslucentDst = renderQueueTranslucentSrc ? GetRenderQueue(static_cast<pragma::ecs::CBaseEntity *>(entWorld)->GetRenderComponent()->GetSceneRenderPass(), true) : nullptr;
				  if(renderQueueTranslucentDst == nullptr || renderQueueTranslucentSrc->queue.empty())
					  continue;
				  renderQueueTranslucentDst->queue.reserve(renderQueueTranslucentDst->queue.size() + renderQueueTranslucentSrc->queue.size());
				  renderQueueTranslucentDst->sortedItemIndices.reserve(renderQueueTranslucentDst->queue.size());
				  auto &pose = entWorld->GetPose();
				  for(auto i = decltype(renderQueueTranslucentSrc->queue.size()) {0u}; i < renderQueueTranslucentSrc->queue.size(); ++i) {
					  auto &item = renderQueueTranslucentSrc->queue.at(i);
					  if(ShouldCull(*renderC, item.mesh, fShouldCull))
						  continue;
					  renderQueueTranslucentDst->queue.push_back(item);
					  renderQueueTranslucentDst->sortedItemIndices.push_back(renderQueueTranslucentSrc->sortedItemIndices.at(i));
					  renderQueueTranslucentDst->sortedItemIndices.back().first = renderQueueTranslucentDst->queue.size() - 1;

					  auto &renderMeshes = renderC->GetRenderMeshes();
					  if(item.mesh >= renderMeshes.size())
						  continue;
					  auto pos = pose * renderMeshes[item.mesh]->GetCenter();
					  renderQueueTranslucentDst->queue.back().sortingKey.SetDistance(pos, cam);
				  }
			  }
		  }
		  m_worldMeshVisibility.resize(m_worldRenderQueues.size());
		  m_worldRenderQueuesReady = true;

		  if(stats)
			  (*stats)->AddTime(pragma::rendering::RenderQueueBuilderStats::Timer::WorldQueueUpdate, std::chrono::steady_clock::now() - t);

		  if(pragma::math::is_flag_set(drawSceneInfo.renderFlags, pragma::rendering::RenderFlags::Dynamic)) {
			  if(stats)
				  t = std::chrono::steady_clock::now();

			  // Now we just need the remaining entities, for which we'll use the scene octree
			  auto *culler = m_scene.FindOcclusionCuller<pragma::COcclusionCullerComponent>();
			  if(culler) {
				  // Some entities are exempt from occlusion culling altogether, we'll handle them here
				  for(auto *pRenderComponent : pragma::CRenderComponent::GetEntitiesExemptFromOcclusionCulling()) {
					  if(ShouldConsiderEntity(static_cast<pragma::ecs::CBaseEntity &>(pRenderComponent->GetEntity()), m_scene, drawSceneInfo.renderFlags, renderMask) == false)
						  continue;
					  AddRenderMeshesToRenderQueue(&rasterizer, drawSceneInfo.renderFlags, *pRenderComponent, m_scene, cam, vp, nullptr);
				  }

				  auto &dynOctree = culler->GetOcclusionOctree();
				  CollectRenderMeshesFromOctree(&rasterizer, drawSceneInfo.renderFlags, drawSceneInfo.clipPlane.has_value(), dynOctree, m_scene, cam, vp, renderMask, frustumPlanes, &bspTrees, &bspLeafNodes);
			  }
			  else {
				  // No occlusion culler available; We'll have to iterate ALL renderable entities
				  pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
				  entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
				  for(auto *ent : entIt) {
					  if(ShouldConsiderEntity(*static_cast<pragma::ecs::CBaseEntity *>(ent), m_scene, drawSceneInfo.renderFlags, renderMask) == false)
						  continue;
					  AddRenderMeshesToRenderQueue(&rasterizer, drawSceneInfo.renderFlags, *static_cast<pragma::ecs::CBaseEntity *>(ent)->GetRenderComponent(), m_scene, cam, vp, nullptr);
				  }
			  }

			  if(stats)
				  (*stats)->AddTime(pragma::rendering::RenderQueueBuilderStats::Timer::OctreeProcessing, std::chrono::steady_clock::now() - t);
		  }
	  },
	  [this, &drawSceneInfo]() {
		  auto *stats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->renderQueueBuilderStats : nullptr;
		  std::chrono::steady_clock::time_point t;
		  if(stats)
			  t = std::chrono::steady_clock::now();

		  // All render queues (aside from world render queues) need to be sorted
		  for(auto &renderQueue : m_renderQueues) {
			  if(stats)
				  t = std::chrono::steady_clock::now();
			  renderQueue->Sort();
			  if(stats)
				  (*stats)->AddTime(pragma::rendering::RenderQueueBuilderStats::Timer::QueueSort, std::chrono::steady_clock::now() - t);

			  if(stats)
				  t = std::chrono::steady_clock::now();
			  BuildRenderQueueInstanceLists(*renderQueue);
			  if(stats)
				  (*stats)->AddTime(pragma::rendering::RenderQueueBuilderStats::Timer::QueueInstancing, std::chrono::steady_clock::now() - t);
			  renderQueue->Unlock();
		  }
		  // pragma::get_cgame()->StopProfilingStage(pragma::CGame::CPUProfilingPhase::BuildRenderQueue);
	  });

	if(fBuildAdditionalQueues)
		fBuildAdditionalQueues(); // Any additional render queues will be processed in parallel to the above

	// As the last operation, we'll wait until all render queues have been built.
	// No further operations must be appended to the render queue builder after this!
	pragma::get_cgame()->GetRenderQueueBuilder().Append(
	  [this, &rasterizer, &cam, posCam, &drawSceneInfo, renderMask]() {
		  auto *stats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->renderQueueBuilderStats : nullptr;
		  std::chrono::steady_clock::time_point t;
		  if(stats)
			  t = std::chrono::steady_clock::now();
		  pragma::get_cgame()->GetRenderQueueWorkerManager().WaitForCompletion();
		  if(stats)
			  (*stats)->AddTime(pragma::rendering::RenderQueueBuilderStats::Timer::WorkerWait, std::chrono::steady_clock::now() - t);
	  },
	  [&drawSceneInfo, tStart]() {
		  // Final completion function. Collect stats and decrease active thread count.
		  auto *stats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->renderQueueBuilderStats : nullptr;
		  std::chrono::steady_clock::time_point t;
		  if(stats)
			  t = std::chrono::steady_clock::now();
		  if(stats) {
			  (*stats)->AddTime(pragma::rendering::RenderQueueBuilderStats::Timer::TotalExecution, std::chrono::steady_clock::now() - tStart);
			  auto &queueWorkerManager = pragma::get_cgame()->GetRenderQueueWorkerManager();
			  auto numWorkers = queueWorkerManager.GetWorkerCount();
			  for(auto i = decltype(numWorkers) {0u}; i < numWorkers; ++i)
				  queueWorkerManager.GetWorker(i).SetStats(nullptr);
		  }
		  --g_activeRenderQueueThreads;
	  });
}
