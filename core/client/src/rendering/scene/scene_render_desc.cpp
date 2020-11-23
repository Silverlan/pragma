/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_scene_component.hpp"
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
#include "pragma/rendering/render_queue.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include <sharedutils/util_shaderinfo.hpp>
#include <sharedutils/alpha_mode.hpp>
#include <pragma/entities/entity_iterator.hpp>

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
	const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull
)
{
	auto &mdlC = renderC.GetModelComponent();
	auto &renderMeshes = renderC.GetRenderMeshes();
	auto renderMode = renderC.GetRenderMode();
	auto first = false;
	for(auto meshIdx=decltype(renderMeshes.size()){0u};meshIdx<renderMeshes.size();++meshIdx)
	{
		if(fShouldCull && ShouldCull(renderC,meshIdx,fShouldCull))
			continue;
		auto &renderMesh = renderMeshes.at(meshIdx);
		auto *mat = mdlC->GetRenderMaterial(renderMesh->GetSkinTextureIndex());
		auto *shader = mat ? dynamic_cast<pragma::ShaderTextured3DBase*>(mat->GetPrimaryShader().get()) : nullptr;
		if(shader == nullptr)
			continue;
		auto nonOpaque = mat->GetAlphaMode() != AlphaMode::Opaque;
		if(nonOpaque && umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Translucent) == false)
			continue;
		auto *renderQueue = getRenderQueue(renderMode,nonOpaque);
		if(renderQueue == nullptr)
			continue;
		if(first == false)
		{
			first = true;
			renderC.UpdateRenderDataMT(drawSceneInfo.commandBuffer,scene,cam,vp);
		}
		renderQueue->Add(static_cast<CBaseEntity&>(renderC.GetEntity()),meshIdx,*mat,*shader,nonOpaque ? &cam : nullptr);
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
	Vector3 min,max;
	renderC.GetRenderBounds(&min,&max);
	auto &pos = renderC.GetEntity().GetPosition();
	min += pos;
	max += pos;
	return fShouldCull(min,max);
}
bool SceneRenderDesc::ShouldCull(pragma::CRenderComponent &renderC,pragma::RenderMeshIndex meshIdx,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull)
{
	auto &renderMeshes = renderC.GetRenderMeshes();
	if(meshIdx >= renderMeshes.size())
		return false;
	auto &renderMesh = renderMeshes.at(meshIdx);
	Vector3 min,max;
	renderMesh->GetBounds(min,max);
	auto &pos = renderC.GetEntity().GetPosition();
	min += pos;
	max += pos;
	return fShouldCull(min,max);
}
bool SceneRenderDesc::ShouldCull(const Vector3 &min,const Vector3 &max,const std::vector<Plane> &frustumPlanes)
{
	return Intersection::AABBInPlaneMesh(min,max,frustumPlanes) == Intersection::Intersect::Outside;
}

void SceneRenderDesc::CollectRenderMeshesFromOctree(
	const util::DrawSceneInfo &drawSceneInfo,const OcclusionOctree<CBaseEntity*> &tree,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,FRender renderFlags,
	const std::function<pragma::rendering::RenderQueue*(RenderMode,bool)> &getRenderQueue,
	const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull,const std::vector<util::BSPTree::Node*> *bspLeafNodes
)
{
	std::function<void(const OcclusionOctree<CBaseEntity*>::Node &node)> iterateTree = nullptr;
	iterateTree = [&iterateTree,&scene,&cam,renderFlags,fShouldCull,&drawSceneInfo,&getRenderQueue,&vp,bspLeafNodes](const OcclusionOctree<CBaseEntity*>::Node &node) {
		auto &nodeBounds = node.GetWorldBounds();
		if(fShouldCull && fShouldCull(nodeBounds.first,nodeBounds.second))
			return;
		if(bspLeafNodes)
		{
			auto hasIntersection = false;
			for(auto *node : *bspLeafNodes)
			{
				if(Intersection::AABBAABB(nodeBounds.first,nodeBounds.second,node->minVisible,node->maxVisible) == Intersection::Intersect::Outside)
					continue;
				hasIntersection = true;
				break;
			}
			if(hasIntersection == false)
				return;
		}
		auto &objs = node.GetObjects();
		for(auto *ent : objs)
		{
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
			if(!renderC || renderC->IsExemptFromOcclusionCulling() || ShouldConsiderEntity(*static_cast<CBaseEntity*>(ent),scene,cam.GetEntity().GetPosition(),renderFlags) == false)
				continue;
			if(fShouldCull && ShouldCull(*renderC,fShouldCull))
				continue;
			AddRenderMeshesToRenderQueue(drawSceneInfo,*renderC,getRenderQueue,scene,cam,vp,fShouldCull);
		}
		auto *children = node.GetChildren();
		if(children == nullptr)
			return;
		for(auto &c : *children)
			iterateTree(static_cast<OcclusionOctree<CBaseEntity*>::Node&>(*c));
	};
	iterateTree(tree.GetRootNode());
}
void SceneRenderDesc::CollectRenderMeshesFromOctree(
	const util::DrawSceneInfo &drawSceneInfo,const OcclusionOctree<CBaseEntity*> &tree,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,FRender renderFlags,
	const std::vector<Plane> &frustumPlanes,const std::vector<util::BSPTree::Node*> *bspLeafNodes
)
{
	CollectRenderMeshesFromOctree(drawSceneInfo,tree,scene,cam,vp,renderFlags,[this](RenderMode renderMode,bool translucent) {return GetRenderQueue(renderMode,translucent);},
	[&frustumPlanes](const Vector3 &min,const Vector3 &max) -> bool {
		return Intersection::AABBInPlaneMesh(min,max,frustumPlanes) == Intersection::Intersect::Outside;
	},bspLeafNodes);
}

bool SceneRenderDesc::ShouldConsiderEntity(CBaseEntity &ent,const pragma::CSceneComponent &scene,const Vector3 &camOrigin,FRender renderFlags)
{
	if(ent.IsInScene(scene) == false || !ent.GetRenderComponent())
		return false;
	auto *renderC = ent.GetRenderComponent();
	auto renderMode = renderC->GetRenderMode();
	return umath::is_flag_set(renderFlags,render_mode_to_render_flag(renderMode)) && ent.GetModel() != nullptr && renderC->ShouldDraw(camOrigin);
}

struct DebugFreezeCamData
{
	Vector3 pos;
	std::vector<Plane> frustumPlanes;
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

static auto cvDrawWorld = GetClientConVar("render_draw_world");
void SceneRenderDesc::BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &hCam = m_scene.GetActiveCamera();
	if(hCam.expired())
		return;

	auto &cam = *hCam;
	// c_game->StartProfilingStage(CGame::CPUProfilingPhase::BuildRenderQueue);
	auto &posCam = g_debugFreezeCamData.has_value() ? g_debugFreezeCamData->pos : cam.GetEntity().GetPosition();

	for(auto &renderQueue : m_renderQueues)
	{
		renderQueue->Clear();
		renderQueue->Lock();
	}
	m_worldRenderQueues.clear();

	m_worldRenderQueuesReady = false;
	c_game->GetRenderQueueBuilder().Append([this,&cam,posCam,&drawSceneInfo]() {
		auto &frustumPlanes = g_debugFreezeCamData.has_value() ? g_debugFreezeCamData->frustumPlanes : cam.GetFrustumPlanes();
		auto fShouldCull = [&frustumPlanes](const Vector3 &min,const Vector3 &max) -> bool {return SceneRenderDesc::ShouldCull(min,max,frustumPlanes);};
		auto vp = cam.GetProjectionMatrix() *cam.GetViewMatrix();

		std::vector<util::BSPTree::Node*> bspLeafNodes;
		// Note: World geometry is handled differently than other entities. World entities have their
		// own pre-built render queues, which we only have to iterate for maximum efficiency. Whether or not a world mesh is culled from the
		// camera frustum is stored in 'm_worldMeshVisibility', which is simply a boolean array so we don't have to copy any
		// data between render queues. (The data in 'm_worldMeshVisibility' is only valid for this render pass.)
		// Translucent world meshes still need to be sorted with other entity meshes, so they are just copied over to the
		// main render queue.
		EntityIterator entItWorld {*c_game};
		entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
		bspLeafNodes.reserve(entItWorld.GetCount());
		m_worldMeshVisibility.reserve(entItWorld.GetCount());
		for(auto *entWorld : entItWorld)
		{
			if(ShouldConsiderEntity(*static_cast<CBaseEntity*>(entWorld),m_scene,posCam,drawSceneInfo.renderFlags) == false)
				continue;
			auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
			auto &bspTree = worldC->GetBSPTree();
			auto *node = bspTree ? bspTree->FindLeafNode(posCam) : nullptr;
			if(node == nullptr)
				continue;
			bspLeafNodes.push_back(node);

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
					auto &pos = pose *renderMeshes.at(item.mesh)->GetCenter();
					renderQueueTranslucentDst->queue.back().sortingKey.SetDistance(pos,cam);
				}
			}
		}
		m_worldMeshVisibility.resize(m_worldRenderQueues.size());
		m_worldRenderQueuesReady = true;

		if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Dynamic))
		{
			// Some entities are exempt from occlusion culling altogether, we'll handle them here
			for(auto *pRenderComponent : pragma::CRenderComponent::GetEntitiesExemptFromOcclusionCulling())
			{
				if(ShouldConsiderEntity(static_cast<CBaseEntity&>(pRenderComponent->GetEntity()),m_scene,posCam,drawSceneInfo.renderFlags) == false)
					continue;
				AddRenderMeshesToRenderQueue(drawSceneInfo,*pRenderComponent,m_scene,cam,vp,nullptr);
			}

			// Now we just need the remaining entities, for which we'll use the scene octree
			auto *culler = m_scene.FindOcclusionCuller();
			if(culler)
			{
				auto &dynOctree = culler->GetOcclusionOctree();
				CollectRenderMeshesFromOctree(drawSceneInfo,dynOctree,m_scene,cam,vp,drawSceneInfo.renderFlags,frustumPlanes,&bspLeafNodes);
			}
		}

		// All render queues (aside from world render queues) need to be sorted
		for(auto &renderQueue : m_renderQueues)
		{
			renderQueue->Sort();
			renderQueue->Unlock();
		}
		// c_game->StopProfilingStage(CGame::CPUProfilingPhase::BuildRenderQueue);
	});
}
#pragma optimize("",on)
