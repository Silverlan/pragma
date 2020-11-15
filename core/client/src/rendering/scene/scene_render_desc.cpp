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
void SceneRenderDesc::PerformOcclusionCulling()
{
	if(m_scene.m_renderer == nullptr || m_scene.m_renderer->IsRasterizationRenderer() == false)
		return;
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::OcclusionCulling);
	GetOcclusionCullingHandler().PerformCulling(m_scene,static_cast<pragma::rendering::RasterizationRenderer&>(*m_scene.m_renderer),GetCulledParticles());

	auto &renderMeshes = GetCulledMeshes();
	GetOcclusionCullingHandler().PerformCulling(m_scene,static_cast<pragma::rendering::RasterizationRenderer&>(*m_scene.m_renderer),renderMeshes);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::OcclusionCulling);
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
void SceneRenderDesc::CollectRenderObjects(FRender renderFlags)
{
	// Prepare rendering
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::PrepareRendering);
	auto bGlow = cvDrawGlow->GetBool();
	auto bTranslucent = cvDrawTranslucent->GetBool();
	if((renderFlags &FRender::Skybox) == FRender::Skybox && c_game->IsRenderModeEnabled(RenderMode::Skybox) && cvDrawSky->GetBool() == true)
		PrepareRendering(m_scene,RenderMode::Skybox,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Skybox;

	if((renderFlags &FRender::World) == FRender::World && c_game->IsRenderModeEnabled(RenderMode::World))
		PrepareRendering(m_scene,RenderMode::World,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::World;

	if((renderFlags &FRender::Water) == FRender::Water && c_game->IsRenderModeEnabled(RenderMode::Water) && cvDrawWater->GetBool() == true)
		PrepareRendering(m_scene,RenderMode::Water,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Water;

	auto *pl = c_game->GetLocalPlayer();
	if((renderFlags &FRender::View) == FRender::View && c_game->IsRenderModeEnabled(RenderMode::View) && pl != nullptr && pl->IsInFirstPersonMode() == true && cvDrawView->GetBool() == true)
		PrepareRendering(m_scene,RenderMode::View,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::View;
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::PrepareRendering);
}

pragma::rendering::RenderMeshCollectionHandler &SceneRenderDesc::GetRenderMeshCollectionHandler() {return m_renderMeshCollectionHandler;}
const pragma::rendering::RenderMeshCollectionHandler &SceneRenderDesc::GetRenderMeshCollectionHandler() const {return const_cast<SceneRenderDesc*>(this)->GetRenderMeshCollectionHandler();}

static auto cvDrawWorld = GetClientConVar("render_draw_world");
void SceneRenderDesc::PrepareRendering(pragma::CSceneComponent &scene,RenderMode renderMode,FRender renderFlags,bool bUpdateTranslucentMeshes,bool bUpdateGlowMeshes)
{
	if(m_scene.m_renderer == nullptr || m_scene.m_renderer->IsRasterizationRenderer() == false)
		return;
	auto &renderMeshData = m_renderMeshCollectionHandler.GetRenderMeshData();
	auto it = renderMeshData.find(renderMode);
	if(it == renderMeshData.end())
		it = renderMeshData.insert(std::remove_reference_t<decltype(renderMeshData)>::value_type(renderMode,std::make_shared<pragma::rendering::CulledMeshData>())).first;

	auto &renderInfo = it->second;
	auto &cam = scene.GetActiveCamera();
	auto &posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;

	auto drawWorld = cvDrawWorld->GetInt();
	auto *matLoad = c_game->GetLoadMaterial();
	auto &renderMeshes = GetCulledMeshes();
	auto &glowMeshes = renderInfo->glowMeshes;
	auto &translucentMeshes = renderInfo->translucentMeshes;
	auto &processed = renderInfo->processed;
	auto &containers = renderInfo->containers;
	glowMeshes.clear();
	translucentMeshes.clear();
	processed.clear();
	containers.clear();

	for(auto it=renderMeshes.begin();it!=renderMeshes.end();++it)
	{
		auto &info = *it;
		auto *ent = static_cast<CBaseEntity*>(info.hEntity.get());
		auto isStatic = ent->IsStatic();
		if(
			(umath::is_flag_set(renderFlags,FRender::Static) == false && isStatic) ||
			(umath::is_flag_set(renderFlags,FRender::Dynamic) == false && isStatic == false)
			)
			continue;
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent.expired())
			continue;
		auto rm = pRenderComponent->GetRenderMode();
		if(renderMode == rm)
		{
			auto itProcessed = renderInfo->processed.find(ent);
			if(itProcessed == renderInfo->processed.end())
			{
				auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
				pRenderComponent->UpdateRenderData(drawCmd);//,true);
				pRenderComponent->Render(renderMode);

				auto wpRenderBuffer = pRenderComponent->GetRenderBuffer();
				if(wpRenderBuffer.expired() == false)
				{
					drawCmd->RecordBufferBarrier(
						*wpRenderBuffer.lock(),
						prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::FragmentShaderBit,
						prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
					);
					auto pAnimComponent = ent->GetAnimatedComponent();
					if(pAnimComponent.valid())
					{
						auto wpBoneBuffer = static_cast<pragma::CAnimatedComponent*>(pAnimComponent.get())->GetBoneBuffer();
						if(wpBoneBuffer.expired() == false)
						{
							drawCmd->RecordBufferBarrier(
								*wpBoneBuffer.lock(),
								prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::FragmentShaderBit,
								prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
							);
						}
					}
				}
				processed.insert(std::remove_reference_t<decltype(processed)>::value_type(ent,true));
			}

			auto &mdlComponent = pRenderComponent->GetModelComponent();
			auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
			assert(mdl != nullptr);
			auto *mesh = static_cast<CModelMesh*>(info.mesh);
			auto &meshes = mesh->GetSubMeshes();
			for(auto it=meshes.begin();it!=meshes.end();++it)
			{
				auto *subMesh = static_cast<CModelSubMesh*>(it->get());
				auto skin = mdlComponent->GetSkin();
				auto numSkins = mdl->GetTextureGroups().size();
				if(skin >= numSkins)
					skin = 0;
				auto idxTexture = mdl->GetMaterialIndex(*subMesh,skin);
				auto *mat = (idxTexture.has_value() && mdlComponent.valid()) ? mdlComponent->GetRenderMaterial(*idxTexture) : nullptr;
				if(mat == nullptr)
					mat = static_cast<CMaterial*>(client->GetMaterialManager().GetErrorMaterial());
				/*else
				{
				auto *diffuse = mat->GetDiffuseMap();
				if(diffuse == nullptr || diffuse->texture == nullptr)
				mat = client->GetMaterialManager().GetErrorMaterial();
				}*/
				if(mat != nullptr)
				{
					if(!mat->IsLoaded())
						mat = static_cast<CMaterial*>(matLoad);
					//auto &hMat = materials[idxTexture];
					//if(hMat.IsValid())
					//{
					if(mat != nullptr)// && mat->GetDiffuseMap() != nullptr && static_cast<Texture*>(mat->GetDiffuseMap()->texture) != nullptr &&static_cast<Texture*>(mat->GetDiffuseMap()->texture)->error == false && static_cast<Texture*>(mat->GetDiffuseMap()->texture)->GetTextureID() != 0)
					{
						// Fill glow map
						auto *glowMap = mat->GetGlowMap();
						if(bUpdateGlowMeshes == true)
						{
							if(glowMap != nullptr)
							{
								auto itMat = std::find_if(glowMeshes.begin(),glowMeshes.end(),[&mat](const std::unique_ptr<RenderSystem::MaterialMeshContainer> &m) {
									return (m->material == mat) ? true : false;
									});
								if(itMat == glowMeshes.end())
								{
									glowMeshes.push_back(std::make_unique<RenderSystem::MaterialMeshContainer>(mat));
									itMat = glowMeshes.end() -1;
								}
								auto itEnt = (*itMat)->containers.find(ent);
								if(itEnt == (*itMat)->containers.end())
									itEnt = (*itMat)->containers.emplace(ent,EntityMeshInfo{ent}).first;
								itEnt->second.meshes.push_back(subMesh);
							}
						}
						//
						auto *info = mat->GetShaderInfo();
						if(info != nullptr && info->GetShader() != nullptr)
						{
							auto *base = static_cast<::util::WeakHandle<prosper::Shader>*>(const_cast<util::ShaderInfo*>(info)->GetShader().get())->get();
							prosper::Shader *shader = nullptr;
							if(drawWorld == 2)
								shader = static_cast<pragma::rendering::RasterizationRenderer&>(*m_scene.m_renderer).m_whShaderWireframe.get();
							else if(base != nullptr && base->GetBaseTypeHashCode() == pragma::ShaderTextured3DBase::HASH_TYPE)
								shader = static_cast<pragma::rendering::RasterizationRenderer&>(*m_scene.m_renderer).GetShaderOverride(static_cast<pragma::ShaderTextured3DBase*>(base));
							if(shader != nullptr && shader->GetBaseTypeHashCode() == pragma::ShaderTextured3DBase::HASH_TYPE)
							{
								// Translucent?
								if(mat->GetAlphaMode() == AlphaMode::Blend)
								{
									if(bUpdateTranslucentMeshes == true)
									{
										auto pTrComponent = ent->GetTransformComponent();
										auto pos = subMesh->GetCenter();
										if(pTrComponent.valid())
										{
											uvec::rotate(&pos,pTrComponent->GetOrientation());
											pos += pTrComponent->GetPosition();
										}
										auto distance = uvec::length_sqr(pos -posCam);
										translucentMeshes.push_back(std::make_unique<RenderSystem::TranslucentMesh>(ent,subMesh,mat,shader->GetHandle(),distance));
									}
									continue; // Skip translucent meshes
								}
								//
								ShaderMeshContainer *shaderContainer = nullptr;
								auto itShader = std::find_if(containers.begin(),containers.end(),[shader](const std::unique_ptr<ShaderMeshContainer> &c) {
									return (c->shader.get() == shader) ? true : false;
									});
								if(itShader != containers.end())
									shaderContainer = itShader->get();
								if(shaderContainer == nullptr)
								{
									if(containers.size() == containers.capacity())
										containers.reserve(containers.capacity() +10);
									containers.push_back(std::make_unique<ShaderMeshContainer>(static_cast<pragma::ShaderTextured3DBase*>(shader)));
									shaderContainer = containers.back().get();
								}
								RenderSystem::MaterialMeshContainer *matContainer = nullptr;
								auto itMat = std::find_if(shaderContainer->containers.begin(),shaderContainer->containers.end(),[mat](const std::unique_ptr<RenderSystem::MaterialMeshContainer> &m) {
									return (m->material == mat) ? true : false;
									});
								if(itMat != shaderContainer->containers.end())
									matContainer = itMat->get();
								if(matContainer == nullptr)
								{
									if(shaderContainer->containers.size() == shaderContainer->containers.capacity())
										shaderContainer->containers.reserve(shaderContainer->containers.capacity() +10);
									shaderContainer->containers.push_back(std::make_unique<RenderSystem::MaterialMeshContainer>(mat));
									matContainer = shaderContainer->containers.back().get();
								}
								EntityMeshInfo *entContainer = nullptr;
								auto itEnt = matContainer->containers.find(ent);
								if(itEnt != matContainer->containers.end())
									entContainer = &itEnt->second;
								if(entContainer == nullptr)
									entContainer = &matContainer->containers.emplace(ent,EntityMeshInfo{ent}).first->second;
								entContainer->meshes.push_back(subMesh);
							}
						}
					}
				}
			}
		}
	}
	if(glowMeshes.empty() == false)
		static_cast<pragma::rendering::RasterizationRenderer&>(*m_scene.m_renderer).m_glowInfo.bGlowScheduled = true;
	if(bUpdateTranslucentMeshes == true)
	{
		// Sort translucent meshes by distance
		std::sort(translucentMeshes.begin(),translucentMeshes.end(),[](const std::unique_ptr<RenderSystem::TranslucentMesh> &a,const std::unique_ptr<RenderSystem::TranslucentMesh> &b) {
			return a->distance < b->distance;
		});
	}
}

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
		return !translucent ? RenderQueueId::Skybox : RenderQueueId::Invalid;
	case RenderMode::View:
		return !translucent ? RenderQueueId::View : RenderQueueId::ViewTranslucent;
	case RenderMode::Water:
		return !translucent ? RenderQueueId::Water : RenderQueueId::Invalid;
	case RenderMode::World:
		return !translucent ? RenderQueueId::World : RenderQueueId::WorldTranslucent;
	}
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

void SceneRenderDesc::AddRenderMeshesToRenderQueue(pragma::CRenderComponent &renderC,const std::vector<Plane> *frustumPlanes)
{
	auto &mdlC = renderC.GetModelComponent();
	auto &renderMeshes = renderC.GetRenderMeshes();
	auto renderMode = renderC.GetRenderMode();
	for(auto meshIdx=decltype(renderMeshes.size()){0u};meshIdx<renderMeshes.size();++meshIdx)
	{
		if(frustumPlanes && ShouldCull(renderC,meshIdx,*frustumPlanes))
			continue;
		auto &renderMesh = renderMeshes.at(meshIdx);
		auto *mat = mdlC->GetRenderMaterial(renderMesh->GetSkinTextureIndex());
		auto *shader = mat ? dynamic_cast<pragma::ShaderTextured3DBase*>(mat->GetPrimaryShader().get()) : nullptr;
		if(shader == nullptr)
			continue;
		auto *renderQueue = GetRenderQueue(renderMode,mat->IsTranslucent());
		if(renderQueue == nullptr)
			continue;
		renderQueue->Add(static_cast<CBaseEntity&>(renderC.GetEntity()),meshIdx,*mat,*shader);
	}
}

bool SceneRenderDesc::ShouldCull(CBaseEntity &ent,const std::vector<Plane> &frustumPlanes)
{
	auto &renderC = ent.GetRenderComponent();
	return renderC.expired() || ShouldCull(*renderC,frustumPlanes);
}
bool SceneRenderDesc::ShouldCull(pragma::CRenderComponent &renderC,const std::vector<Plane> &frustumPlanes)
{
	Vector3 min,max;
	renderC.GetRenderBounds(&min,&max);
	auto &pos = renderC.GetEntity().GetPosition();
	min += pos;
	max += pos;
	return Intersection::AABBInPlaneMesh(min,max,frustumPlanes) == Intersection::Intersect::Outside;
}
bool SceneRenderDesc::ShouldCull(pragma::CRenderComponent &renderC,pragma::RenderMeshIndex meshIdx,const std::vector<Plane> &frustumPlanes)
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
	return Intersection::AABBInPlaneMesh(min,max,frustumPlanes) == Intersection::Intersect::Outside;
}

void SceneRenderDesc::CollectRenderMeshesFromOctree(const OcclusionOctree<CBaseEntity*> &tree,pragma::CSceneComponent &scene,FRender renderFlags,const std::vector<Plane> &frustumPlanes,const std::vector<util::BSPTree::Node*> *bspLeafNodes)
{
	std::function<void(const OcclusionOctree<CBaseEntity*>::Node &node)> iterateTree = nullptr;
	iterateTree = [this,&iterateTree,&scene,renderFlags,&frustumPlanes,bspLeafNodes](const OcclusionOctree<CBaseEntity*>::Node &node) {
		auto &nodeBounds = node.GetWorldBounds();
		if(Intersection::AABBInPlaneMesh(nodeBounds.first,nodeBounds.second,frustumPlanes) == Intersection::Intersect::Outside)
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
			auto &renderC = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
			if(renderC.expired() || renderC->IsExemptFromOcclusionCulling() || ShouldConsiderEntity(*static_cast<CBaseEntity*>(ent),scene,renderFlags) == false)
				continue;
			if(ShouldCull(*renderC,frustumPlanes))
				continue;
			AddRenderMeshesToRenderQueue(*renderC,&frustumPlanes);
		}
		auto *children = node.GetChildren();
		if(children == nullptr)
			return;
		for(auto &c : *children)
			iterateTree(static_cast<OcclusionOctree<CBaseEntity*>::Node&>(*c));
	};
	iterateTree(tree.GetRootNode());
}

bool SceneRenderDesc::ShouldConsiderEntity(CBaseEntity &ent,pragma::CSceneComponent &scene,FRender renderFlags)
{
	if(ent.IsInScene(scene) == false || ent.GetRenderComponent().expired())
		return false;
	auto renderMode = ent.GetRenderComponent()->GetRenderMode();
	return umath::is_flag_set(renderFlags,render_mode_to_render_flag(renderMode)) && ent.GetModel() != nullptr;
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

void SceneRenderDesc::BuildRenderQueue(pragma::CSceneComponent &scene,FRender renderFlags)
{
	auto &cam = scene.GetActiveCamera();
	auto &posCam = g_debugFreezeCamData.has_value() ? g_debugFreezeCamData->pos : (cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN);

	auto drawWorld = cvDrawWorld->GetBool();
	if(drawWorld == false)
		umath::set_flag(renderFlags,FRender::World,false);

	for(auto &renderQueue : m_renderQueues)
		renderQueue->Clear();
	m_worldRenderQueues.clear();

	auto &frustumPlanes = g_debugFreezeCamData.has_value() ? g_debugFreezeCamData->frustumPlanes : cam->GetFrustumPlanes();

	// Note: World geometry is handled differently than other entities. World entities have their
	// own pre-built render queues, which we only have to iterate for maximum efficiency. Whether or not a world mesh is culled from the
	// camera frustum is stored in 'm_worldMeshVisibility', which is simply a boolean array so we don't have to copy any
	// data between render queues. (The data in 'm_worldMeshVisibility' is only valid for this render pass.)
	// Translucent world meshes still need to be sorted with other entity meshes, so they are just copied over to the
	// main render queue.
	EntityIterator entItWorld {*c_game};
	entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
	std::vector<util::BSPTree::Node*> bspLeafNodes;
	bspLeafNodes.reserve(entItWorld.GetCount());
	m_worldMeshVisibility.reserve(entItWorld.GetCount());
	for(auto *entWorld : entItWorld)
	{
		if(ShouldConsiderEntity(*static_cast<CBaseEntity*>(entWorld),scene,renderFlags) == false)
			continue;
		auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
		auto &bspTree = worldC->GetBSPTree();
		auto *node = bspTree ? bspTree->FindLeafNode(posCam) : nullptr;
		if(node == nullptr)
			continue;
		bspLeafNodes.push_back(node);
		auto &renderC = static_cast<CBaseEntity&>(worldC->GetEntity()).GetRenderComponent();
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
				worldMeshVisibility.at(item.mesh) = !ShouldCull(*renderC,item.mesh,frustumPlanes);
			}
		}

		// Translucent meshes will have to be sorted dynamically with all other non-world translucent objects,
		// so we'll copy the information to the dynamic queue
		auto *renderQueueTranslucentSrc = worldC->GetClusterRenderQueue(node->cluster,true /* translucent */);
		auto *renderQueueTranslucentDst = renderQueueTranslucentSrc ? GetRenderQueue(static_cast<CBaseEntity*>(entWorld)->GetRenderComponent()->GetRenderMode(),true) : nullptr;
		if(renderQueueTranslucentDst == nullptr)
			continue;
		auto offset = renderQueueTranslucentDst->queue.size();
		renderQueueTranslucentDst->queue.reserve(offset +renderQueueTranslucentSrc->queue.size());
		renderQueueTranslucentDst->sortedItemIndices.reserve(renderQueueTranslucentDst->queue.size());
		for(auto i=decltype(renderQueueTranslucentSrc->queue.size()){0u};i<renderQueueTranslucentSrc->queue.size();++i)
		{
			auto &item = renderQueueTranslucentSrc->queue.at(i);
			if(ShouldCull(*renderC,item.mesh,frustumPlanes))
				continue;
			// TODO: Add distance information
			renderQueueTranslucentDst->queue.push_back(item);
			renderQueueTranslucentDst->sortedItemIndices.push_back(renderQueueTranslucentSrc->sortedItemIndices.at(i));
		}
	}
	m_worldMeshVisibility.resize(m_worldRenderQueues.size());

	// Some entities are exempt from occlusion culling altogether, we'll handle them here
	for(auto *pRenderComponent : pragma::CRenderComponent::GetEntitiesExemptFromOcclusionCulling())
	{
		if(ShouldConsiderEntity(static_cast<CBaseEntity&>(pRenderComponent->GetEntity()),scene,renderFlags) == false)
			continue;
		AddRenderMeshesToRenderQueue(*pRenderComponent);
	}

	// Now we just need the remaining entities, for which we'll use the scene octree
	auto *culler = scene.FindOcclusionCuller();
	if(culler)
	{
		auto &dynOctree = culler->GetOcclusionOctree();
		CollectRenderMeshesFromOctree(dynOctree,scene,renderFlags,frustumPlanes,&bspLeafNodes);
#if 0
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
			if(ShouldExamine(scene,renderer,*ent,bViewModel,cullByViewFrustum ? &planes : nullptr) == false)
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
#endif
	}

	// All render queues (aside from world render queues) need to be sorted
	for(auto &renderQueue : m_renderQueues)
		renderQueue->Sort();
}
#pragma optimize("",on)
