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
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include <sharedutils/util_shaderinfo.hpp>
#include <sharedutils/alpha_mode.hpp>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
SceneRenderDesc::SceneRenderDesc(pragma::CSceneComponent &scene)
	: m_scene{scene}
{
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
void SceneRenderDesc::ReloadOcclusionCullingHandler()
{
	auto occlusionCullingMode = c_game->GetConVarInt("cl_render_occlusion_culling");
	switch(occlusionCullingMode)
	{
	case 1: /* Brute-force */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerBruteForce>();
		break;
	case 2: /* CHC++ */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerCHC>();
		break;
	case 4: /* BSP */
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
	case 3: /* Octtree */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerOctTree>();
		break;
	case 0: /* Off */
	default:
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerInert>();
		break;
	}
	m_occlusionCullingHandler->Initialize();
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
#pragma optimize("",on)
