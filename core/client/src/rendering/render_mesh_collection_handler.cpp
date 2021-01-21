/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/render_mesh_collection_handler.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/c_world.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <pragma/console/convars.h>
#include <prosper_util.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include <cmaterialmanager.h>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;


static auto cvDrawWorld = GetClientConVar("render_draw_world");
const std::vector<pragma::OcclusionMeshInfo> &rendering::RenderMeshCollectionHandler::PerformOcclusionCulling(pragma::CSceneComponent &scene,const CRasterizationRendererComponent &renderer,const Vector3 &posCam,bool cullByViewFrustum)
{
	m_culledMeshes.clear();
	auto *world = static_cast<pragma::CWorldComponent*>(c_game->GetWorld());
	if(world == nullptr)
		return m_culledMeshes;
	auto &occlusionCullingHandler = scene.GetSceneRenderDesc().GetOcclusionCullingHandler();
	occlusionCullingHandler.PerformCulling(scene,renderer,posCam,m_culledMeshes,cullByViewFrustum);
	return m_culledMeshes;
}

const std::vector<pragma::OcclusionMeshInfo> &rendering::RenderMeshCollectionHandler::GetOcclusionFilteredMeshes() const {return const_cast<RenderMeshCollectionHandler*>(this)->GetOcclusionFilteredMeshes();}
std::vector<pragma::OcclusionMeshInfo> &rendering::RenderMeshCollectionHandler::GetOcclusionFilteredMeshes() {return m_culledMeshes;}

const std::vector<pragma::CParticleSystemComponent*> &rendering::RenderMeshCollectionHandler::GetOcclusionFilteredParticleSystems() const {return const_cast<RenderMeshCollectionHandler*>(this)->GetOcclusionFilteredParticleSystems();}
std::vector<pragma::CParticleSystemComponent*> &rendering::RenderMeshCollectionHandler::GetOcclusionFilteredParticleSystems() {return m_culledParticles;}

const std::unordered_map<RenderMode,std::shared_ptr<rendering::CulledMeshData>> &rendering::RenderMeshCollectionHandler::GetRenderMeshData() const {return const_cast<RenderMeshCollectionHandler*>(this)->GetRenderMeshData();}
std::unordered_map<RenderMode,std::shared_ptr<rendering::CulledMeshData>> &rendering::RenderMeshCollectionHandler::GetRenderMeshData() {return m_culledMeshData;}
std::shared_ptr<rendering::CulledMeshData> rendering::RenderMeshCollectionHandler::GetRenderMeshData(RenderMode renderMode) const
{
	auto it = m_culledMeshData.find(renderMode);
	if(it == m_culledMeshData.end())
		return nullptr;
	return it->second;
}

rendering::RenderMeshCollectionHandler::ResultFlags rendering::RenderMeshCollectionHandler::GenerateOptimizedRenderObjectStructures(pragma::CSceneComponent &scene,const CRasterizationRendererComponent &renderer,const Vector3 &posCam,FRender renderFlags,RenderMode renderMode,bool useGlowMeshes,bool useTranslucentMeshes)
{
	auto it = m_culledMeshData.find(renderMode);
	if(it == m_culledMeshData.end())
		return ResultFlags::None;
	auto &renderInfo = it->second;

	auto drawWorld = cvDrawWorld->GetInt();
	auto *matLoad = c_game->GetLoadMaterial();
	auto &renderMeshes = m_culledMeshes;
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
		if(!pRenderComponent)
			continue;
		auto rm = pRenderComponent->GetRenderMode();
		if(renderMode == rm)
		{
			auto itProcessed = renderInfo->processed.find(ent);
			if(itProcessed == renderInfo->processed.end())
			{
				auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
				//pRenderComponent->UpdateRenderData(drawCmd);//,true); // TODO

				/*auto &wpRenderBuffer = pRenderComponent->GetRenderBuffer();
				if(wpRenderBuffer)
				{
					drawCmd->RecordBufferBarrier(
						*wpRenderBuffer,
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
				}*/
				processed.insert(std::remove_reference_t<decltype(processed)>::value_type(ent,true));
			}

			auto *mdlComponent = pRenderComponent->GetModelComponent();
			auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
			assert(mdl != nullptr);
			auto *mesh = static_cast<CModelMesh*>(info.mesh);
			auto &meshes = mesh->GetSubMeshes();
			for(auto it=meshes.begin();it!=meshes.end();++it)
			{
				auto *subMesh = static_cast<CModelSubMesh*>(it->get());
				auto idxTexture = mdl->GetMaterialIndex(*subMesh,mdlComponent->GetSkin());
				auto *mat = (idxTexture.has_value() && mdlComponent) ? mdlComponent->GetRenderMaterial(*idxTexture) : nullptr;
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
						if(useGlowMeshes == true)
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
								shader = renderer.GetWireframeShader();
							else if(base != nullptr && base->GetBaseTypeHashCode() == pragma::ShaderGameWorldLightingPass::HASH_TYPE)
								shader = renderer.GetShaderOverride(static_cast<pragma::ShaderGameWorldLightingPass*>(base));
							if(shader != nullptr && shader->GetBaseTypeHashCode() == pragma::ShaderGameWorldLightingPass::HASH_TYPE)
							{
								// Translucent?
								if(mat->IsTranslucent() == true)
								{
									if(useTranslucentMeshes == true)
									{
										auto pTrComponent = ent->GetTransformComponent();
										auto pos = subMesh->GetCenter();
										if(pTrComponent != nullptr)
										{
											uvec::rotate(&pos,pTrComponent->GetRotation());
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
									containers.push_back(std::make_unique<ShaderMeshContainer>(static_cast<pragma::ShaderGameWorldLightingPass*>(shader)));
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
	auto flags = ResultFlags::None;
	if(glowMeshes.empty() == false)
		flags |= ResultFlags::HasGlowMeshes;
	if(useTranslucentMeshes == true)
	{
		// Sort translucent meshes by distance
		std::sort(translucentMeshes.begin(),translucentMeshes.end(),[](const std::unique_ptr<RenderSystem::TranslucentMesh> &a,const std::unique_ptr<RenderSystem::TranslucentMesh> &b) {
			return a->distance < b->distance;
			});
	}
	return flags;
}

