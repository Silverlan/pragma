/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <cmaterial.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

ShadowRenderer::ShadowRenderer()
{
	m_shader = c_game->GetGameShader(CGame::GameShader::Shadow);
	m_shaderTransparent = c_game->GetGameShader(CGame::GameShader::ShadowTransparent);
	m_shaderSpot = c_game->GetGameShader(CGame::GameShader::ShadowSpot);
	m_shaderSpotTransparent = c_game->GetGameShader(CGame::GameShader::ShadowTransparentSpot);
	m_shaderCSM = c_game->GetGameShader(CGame::GameShader::ShadowCSM);
	m_shaderCSMTransparent = c_game->GetGameShader(CGame::GameShader::ShadowCSMTransparent);

	m_octreeCallbacks.nodeCallback = [this](const OcclusionOctree<std::shared_ptr<ModelMesh>>::Node &node) -> bool {
		auto &bounds = node.GetWorldBounds();
		return umath::intersection::aabb_sphere(bounds.first, bounds.second, m_lightSourceData.position, m_lightSourceData.radius);
	};

	m_octreeCallbacks.entityCallback = [this](const CBaseEntity &ent, uint32_t renderFlags) {
		m_currentEntity = &ent;
		m_currentModel = ent.GetModel().get();
		m_currentRenderFlags = renderFlags;
		if(m_currentModel == nullptr) {
			m_currentEntity = nullptr;
			return;
		}
		m_shadowCasters.push_back({});
		auto &info = m_shadowCasters.back();
		info.entity = &ent;
		info.renderFlags = (m_lightSourceData.type == pragma::LightType::Point) ? renderFlags : 1u; // Spot-lights only have 1 layer, so we can ignore the flags
		auto pRenderComponent = ent.GetRenderComponent();
		if(pRenderComponent) {
			// Make sure entity buffer data is up to date
			// TODO
			//pRenderComponent->UpdateRenderData(m_lightSourceData.drawCmd,{}); // TODO: Camera position
		}
	};

	m_octreeCallbacks.meshCallback = [this](const std::shared_ptr<ModelMesh> &mesh) {
		auto *ent = m_currentEntity;
		if(m_lightSourceData.light->ShouldPass(*ent, *static_cast<CModelMesh *>(mesh.get()), m_currentRenderFlags) == false)
			return;
		for(auto &subMesh : mesh->GetSubMeshes()) {
			if(m_lightSourceData.light->ShouldPass(*m_currentModel, *static_cast<CModelSubMesh *>(subMesh.get())) == false)
				continue;
			auto matIdx = m_currentModel->GetMaterialIndex(*subMesh);
			auto *mat = matIdx.has_value() ? m_currentModel->GetMaterial(*matIdx) : nullptr;
			if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO: Do this properly
				continue;
			m_octreeCallbacks.subMeshCallback(*m_currentModel, *static_cast<CModelSubMesh *>(subMesh.get()), m_currentRenderFlags);
		}
	};

	m_octreeCallbacks.subMeshCallback = [this](const Model &mdl, const CModelSubMesh &subMesh, uint32_t renderFlags) {
		auto matIdx = mdl.GetMaterialIndex(subMesh);
		auto *mat = matIdx.has_value() ? const_cast<Model &>(mdl).GetMaterial(*matIdx) : nullptr;
		m_shadowCasters.push_back({});
		auto &info = m_shadowCasters.back();
		info.mesh = &subMesh;
		info.renderFlags = renderFlags;
		info.material = mat;
	};
}

void ShadowRenderer::UpdateWorldShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light)
{
	auto *scene = light.FindShadowScene();
	if(scene == nullptr)
		return;
	auto *pWorld = c_game->GetWorld();
	if(pWorld == nullptr)
		return;
	auto &entWorld = static_cast<CBaseEntity &>(pWorld->GetEntity());
	if(entWorld.IsInScene(*scene) == false)
		return;
	auto &mdl = entWorld.GetModel();
	auto meshTree = mdl ? static_cast<pragma::CWorldComponent *>(pWorld)->GetMeshTree() : nullptr;
	if(meshTree == nullptr)
		return;

	uint32_t renderFlags = 0u;
	if(light.ShouldPass(entWorld, renderFlags) == false)
		return;

	// Iterate all meshes in the world and populate m_shadowCasters
	m_octreeCallbacks.entityCallback(entWorld, renderFlags);
	meshTree->IterateObjects(m_octreeCallbacks.nodeCallback, m_octreeCallbacks.meshCallback);
}
void ShadowRenderer::UpdateEntityShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light)
{
	auto *scene = light.FindShadowScene();
	if(scene == nullptr)
		return;
	auto *culler = scene->FindOcclusionCuller();
	if(culler == nullptr)
		return;
	auto &octree = culler->GetOcclusionOctree();
	// Iterate all entities in the scene and populate m_shadowCasters
	octree.IterateObjects(
	  [this](const OcclusionOctree<CBaseEntity *>::Node &node) -> bool {
		  auto &bounds = node.GetWorldBounds();
		  return umath::intersection::aabb_sphere(bounds.first, bounds.second, m_lightSourceData.position, m_lightSourceData.radius);
	  },
	  [this, &light, &drawCmd, scene](const CBaseEntity *ent) {
		  auto pRenderComponent = ent->GetRenderComponent();
		  if(!pRenderComponent || ent->IsInScene(*scene) == false || pRenderComponent->ShouldDrawShadow() == false || ent->IsWorld() == true)
			  return;
		  uint32_t renderFlags = 0;
		  if(light.ShouldPass(*ent, renderFlags) == false)
			  return;
		  m_octreeCallbacks.entityCallback(*ent, renderFlags);
		  auto *mdlComponent = pRenderComponent->GetModelComponent();
		  if(mdlComponent) {
			  auto mdl = mdlComponent->GetModel();
			  for(auto &mesh : static_cast<pragma::CModelComponent &>(*mdlComponent).GetLODMeshes()) {
				  if(light.ShouldPass(*ent, *static_cast<CModelMesh *>(mesh.get()), renderFlags) == false)
					  continue;
				  //meshCallback(static_cast<CModelMesh*>(mesh.get()),renderFlags);
				  for(auto &subMesh : mesh->GetSubMeshes()) {
					  if(light.ShouldPass(*mdl, *static_cast<CModelSubMesh *>(subMesh.get())) == false)
						  continue;
					  auto matIdx = mdl->GetMaterialIndex(*subMesh);
					  auto *mat = matIdx.has_value() ? mdl->GetMaterial(*matIdx) : nullptr;
					  if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO
						  continue;
					  m_octreeCallbacks.subMeshCallback(*mdl, *static_cast<CModelSubMesh *>(subMesh.get()), renderFlags);
				  }
			  }
		  }
	  });
}

bool ShadowRenderer::UpdateShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light, pragma::CLightComponent::ShadowMapType smType)
{
	// TODO: Remove me
#if 0
	m_shadowCasters.clear();
	auto hShadowMap = light.GetShadowMap(smType);
	if(hShadowMap.expired())
		return false;
	auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	if(hShadowMap->GetLastFrameRendered() == frameId)
		return false;
	auto wpRt = hShadowMap->RequestRenderTarget();
	if(wpRt.expired() == true)
	{
		light.SetShadowMapIndex(std::numeric_limits<uint32_t>::max(),smType);
		return false;
	}
	auto rt = wpRt.lock();
	light.SetShadowMapIndex(rt->index,smType);
	hShadowMap->SetLastFrameRendered(frameId);

	if(hShadowMap->IsDirty() == false)
	{
		if(smType == pragma::CLightComponent::ShadowMapType::Static)
		{
			// Skip rendering; We've already rendered the static shadow map, no need to do it again since nothing has changed.
			return false;
		}
		// Dynamic shadow map will ALWAYS be re-rendered
	}
	hShadowMap->SetDirty(false);

	auto &ent = light.GetEntity();
	auto pRadiusComponent = ent.GetComponent<pragma::CRadiusComponent>();
	if(pRadiusComponent.expired())
		return false;
	light.GetLight(m_lightSourceData.type);
	m_lightSourceData.drawCmd = drawCmd;
	m_lightSourceData.light = &light;
	m_lightSourceData.position = ent.GetPosition();
	m_lightSourceData.radius = pRadiusComponent->GetRadius();
	// TODO: Also render static props here!
	switch(smType)
	{
	case pragma::CLightComponent::ShadowMapType::Static:
		UpdateWorldShadowCasters(drawCmd,light);
		break;
	case pragma::CLightComponent::ShadowMapType::Dynamic:
		UpdateEntityShadowCasters(drawCmd,light);
		break;
	}
#endif
	return true;
}

ShadowRenderer::RenderResultFlags ShadowRenderer::RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light, uint32_t layerId, const Mat4 &depthMVP, pragma::ShaderShadow &shader, bool bTranslucent)
{
	// TODO
#if 0
	auto bRetTranslucent = false;
	if(shader.BeginDraw(drawCmd) == false)
		return ShadowRenderer::RenderResultFlags::None;
	auto hasTranslucents = false;
	if(shader.BindLight(light) == true)
	{
		auto layerFlag = 1<<layerId;
		auto bProcessMeshes = false;
		Material *prevMat = nullptr;
		for(auto &info : m_shadowCasters)
		{
			if(info.entity != nullptr)
			{
				bProcessMeshes = ((info.renderFlags &layerFlag) != 0) ? true : false;
				if(bProcessMeshes == true)
					shader.BindEntity(*const_cast<CBaseEntity*>(info.entity),depthMVP);
			}
			if(info.mesh != nullptr && bProcessMeshes == true)
			{
				if(info.material != nullptr && info.material->IsTranslucent())
				{
					if(bTranslucent == true)
					{
						auto &shaderTranslucent = static_cast<pragma::ShaderShadowTransparent&>(shader);
						if(info.material == prevMat || shaderTranslucent.BindMaterial(static_cast<CMaterial&>(*info.material)) == true)
						{
							shaderTranslucent.Draw(*const_cast<CModelSubMesh*>(info.mesh));
							prevMat = info.material;
						}
					}
					else
						hasTranslucents = true;
				}
				else if(bTranslucent == false)
					shader.Draw(*const_cast<CModelSubMesh*>(info.mesh));
			}
		}
	}
	shader.EndDraw();
	return hasTranslucents ? ShadowRenderer::RenderResultFlags::TranslucentPending : ShadowRenderer::RenderResultFlags::None;
#endif
	return RenderResultFlags::None;
}

static CVar cvParticleQuality = GetClientConVar("cl_render_particle_quality");
void ShadowRenderer::RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light, pragma::CLightComponent::ShadowMapType smType, pragma::LightType type, bool drawParticleShadows)
{
	auto hShadowMap = light.GetShadowMap(smType);
	if(hShadowMap.expired() || light.GetEffectiveShadowType() == pragma::BaseEnvLightComponent::ShadowType::None || UpdateShadowCasters(drawCmd, light, smType) == false)
		return;
	auto &shader = (type != pragma::LightType::Spot) ? static_cast<pragma::ShaderShadow &>(*m_shader.get()) : static_cast<pragma::ShaderShadow &>(*m_shaderSpot.get());
	pragma::ShaderShadowTransparent *shaderTransparent = nullptr;
	if(type != pragma::LightType::Spot)
		shaderTransparent = static_cast<pragma::ShaderShadowTransparent *>(m_shaderTransparent.expired() == false ? m_shaderTransparent.get() : nullptr);
	else
		shaderTransparent = static_cast<pragma::ShaderShadowTransparent *>(m_shaderSpotTransparent.expired() == false ? m_shaderSpotTransparent.get() : nullptr);
#if DEBUG_SHADOWS == 1
	s_shadowMeshCount = 0;
	s_shadowIndexCount = 0;
	s_shadowTriangleCount = 0;
	s_shadowVertexCount = 0;
#endif

	auto *smRt = hShadowMap->GetDepthRenderTarget();
	auto &tex = smRt->GetTexture();
	auto *scene = c_game->GetScene();
	auto *renderer = scene ? scene->GetRenderer() : nullptr;
	auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	if(raster.expired())
		return;

	auto &img = tex.GetImage();
	auto numLayers = hShadowMap->GetLayerCount();
	drawCmd->RecordImageBarrier(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

	for(auto layerId = decltype(numLayers) {0}; layerId < numLayers; ++layerId) {
		auto &depthMVP = light.GetTransformationMatrix(layerId);
		auto *framebuffer = hShadowMap->GetFramebuffer(layerId);

		const prosper::ClearValue clearVal {prosper::ClearDepthStencilValue {1.f}};
		if(drawCmd->RecordBeginRenderPass(*smRt, layerId, prosper::IPrimaryCommandBuffer::RenderPassFlags::None, &clearVal) == false)
			continue;
		// TODO
		//auto renderResultFlags = RenderShadows(drawCmd,light,layerId,depthMVP,shader,false);
		//if(umath::is_flag_set(renderResultFlags,RenderResultFlags::TranslucentPending) && shaderTransparent != nullptr)
		//	RenderShadows(drawCmd,light,layerId,depthMVP,shader,true); // Draw translucent shadows
		if(drawParticleShadows == true && renderer) {
			auto *scene = c_game->GetRenderScene();
			// TODO: Only culled particles
			EntityIterator entIt {*c_game};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
			for(auto *ent : entIt) {
				auto p = ent->GetComponent<pragma::CParticleSystemComponent>();
				if(p.valid() && p->GetCastShadows() == true)
					p->RecordRenderShadow(*drawCmd, *scene, *raster, &light, layerId);
			}
		}
		drawCmd->RecordEndRenderPass();

		prosper::util::ImageSubresourceRange range {layerId};
		drawCmd->RecordPostRenderPassImageBarrier(img, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, range);
	}
}

void ShadowRenderer::RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light)
{
	auto type = pragma::LightType::Undefined;
	auto *pLight = light.GetLight(type);
	auto bDrawParticleShadows = (cvParticleQuality->GetInt() >= 3) ? true : false;
	if(type == pragma::LightType::Directional) {
		RenderCSMShadows(drawCmd, static_cast<pragma::CLightDirectionalComponent &>(*pLight), bDrawParticleShadows);
		return;
	}
	if(m_shader.expired() || m_shaderSpot.expired())
		return;
	RenderShadows(drawCmd, light, pragma::CLightComponent::ShadowMapType::Static, type, bDrawParticleShadows);
	RenderShadows(drawCmd, light, pragma::CLightComponent::ShadowMapType::Dynamic, type, bDrawParticleShadows);
}
