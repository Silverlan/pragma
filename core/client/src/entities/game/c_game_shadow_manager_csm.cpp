/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/model/c_model.h>
#include <pragma/model/c_modelmesh.h>
#include <prosper_command_buffer.hpp>
#include <image/prosper_texture.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void ShadowRenderer::RenderCSMShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightDirectionalComponent &light, bool drawParticleShadows)
{
#if 0
	auto pLightComponent = light.GetEntity().GetComponent<pragma::CLightComponent>();
	auto *shadowScene = pLightComponent.valid() ? pLightComponent->FindShadowScene() : nullptr;
	// TODO
	auto hShadowMap = light.GetEntity().GetComponent<CShadowCSMComponent>();
	if(hShadowMap.expired() || m_shaderCSM.expired() || shadowScene == nullptr)// || m_shaderCSMTransparent.expired()) // prosper TODO: Transparent shadow shader
		return;
	auto &shaderCsm = static_cast<pragma::ShaderShadowCSM&>(*m_shaderCSM);
	auto *shaderCsmTransparent = static_cast<pragma::ShaderShadowCSMTransparent*>(m_shaderCSMTransparent.expired() == false ? m_shaderCSMTransparent.get() : nullptr);
	auto *cam = c_game->GetRenderCamera();
	auto &posCam = cam ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	auto &csm = *hShadowMap;

	auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	if(csm.GetLastUpdateFrameId() == frameId)
		return;
	csm.SetLastUpdateFrameId(frameId);

	const std::array<pragma::CLightComponent::ShadowMapType,1> renderPasses = {pragma::CLightComponent::ShadowMapType::Dynamic};
	auto &staticDepthImg = csm.GetDepthTexture(pragma::CLightComponent::ShadowMapType::Static)->GetImage();
	csm.RenderBatch(drawCmd,light);

	// Render dynamic objects
	for(auto rp : renderPasses)
	{
		if(pLightComponent->ShouldUpdateRenderPass(rp) == false)
			continue;
		auto *tex = csm.GetDepthTexture(rp);
		if(tex == nullptr)
			continue;
		auto bDynamic = (rp == pragma::CLightComponent::ShadowMapType::Dynamic) ? true : false;
		if(bDynamic == true)
			;//staticDepthImg->SetDrawLayout(prosper::ImageLayout::TransferSrcOptimal);
		else
			;//pLightComponent->SetStaticResolved(true);

		auto &rt = csm.GetRenderTarget(rp);
		auto *renderPass = csm.GetRenderPass(rp);
		auto &img = tex->GetImage();

		auto numLayers = csm.GetLayerCount();

		auto clearValues = {
			prosper::ClearValue{prosper::ClearDepthStencilValue{1.f,0}} // Depth Attachment
		};

		m_shadowCasters.clear();

		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
		for(auto *ent : entIt)
		{
			auto pRenderComponent = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
			if(ent->IsInert() == true || static_cast<CBaseEntity*>(ent)->IsInScene(*shadowScene) || pRenderComponent->ShouldDrawShadow(posCam) == false)
				continue;
			auto *mdlComponent = pRenderComponent->GetModelComponent();
			auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
			if(mdl == nullptr)
				continue;
			uint32_t renderFlags = 0;
			if(pLightComponent->ShouldPass(static_cast<CBaseEntity&>(*ent),renderFlags) == false)
				continue;
			auto &lodMeshes = pRenderComponent->GetLODMeshes();
			if(lodMeshes.empty())
				continue;

			if(m_shadowCasters.size() == m_shadowCasters.capacity())
				m_shadowCasters.reserve(m_shadowCasters.size() +100);
			m_shadowCasters.push_back({});
			auto &info = m_shadowCasters.back();
			info.entity = static_cast<CBaseEntity*>(ent);
			info.renderFlags = renderFlags;

			for(auto &mesh : lodMeshes)
			{
				auto meshRenderFlags = 0u;
				if(pLightComponent->ShouldPass(static_cast<CBaseEntity&>(*ent),*static_cast<CModelMesh*>(mesh.get()),meshRenderFlags) == false)
					continue;
				for(auto &subMesh : mesh->GetSubMeshes())
				{
					auto matIdx = mdl->GetMaterialIndex(*subMesh);
					auto *mat = matIdx.has_value() ? const_cast<Model&>(*mdl).GetMaterial(*matIdx) : nullptr;
					if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO
						continue;
					if(m_shadowCasters.size() == m_shadowCasters.capacity())
						m_shadowCasters.reserve(m_shadowCasters.size() +100);
					m_shadowCasters.push_back({});
					auto &info = m_shadowCasters.back();
					info.mesh = static_cast<CModelSubMesh*>(subMesh.get());
					info.renderFlags = meshRenderFlags;
					info.material = mat;
				}
			}
		}

		auto bHasTranslucent = false;
		for(auto layer=decltype(numLayers){0};layer<numLayers;++layer)
		{
			drawCmd->RecordImageBarrier(img,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal,layer);

			prosper::util::ClearImageInfo clearImageInfo {};
			clearImageInfo.subresourceRange.baseArrayLayer = layer;
			clearImageInfo.subresourceRange.layerCount = 1u;
			drawCmd->RecordClearImage(img,prosper::ImageLayout::TransferDstOptimal,1.f,clearImageInfo); // Clear this layer
			auto framebuffer = csm.GetFramebuffer(rp,layer);

			drawCmd->RecordImageBarrier(img,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal,layer);

			if(bDynamic == true)
			{
				drawCmd->RecordImageBarrier(staticDepthImg,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferSrcOptimal,layer);
				drawCmd->RecordImageBarrier(img,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::TransferDstOptimal,layer);

				prosper::util::BlitInfo blitInfo {};
				blitInfo.srcSubresourceLayer.baseArrayLayer = blitInfo.dstSubresourceLayer.baseArrayLayer = layer;
				blitInfo.srcSubresourceLayer.layerCount = blitInfo.dstSubresourceLayer.layerCount = 1u;
				drawCmd->RecordBlitImage(blitInfo,staticDepthImg,img);

				drawCmd->RecordImageBarrier(staticDepthImg,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal,layer);
				drawCmd->RecordImageBarrier(img,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal,layer);
			}

			// Update entity render buffers (has to be done before render pass has started)
			const auto layerFlag = 1<<layer;
			for(auto &info : m_shadowCasters)
			{
				if(info.entity != nullptr)
				{
					auto bProcessMeshes = ((info.renderFlags &layerFlag) != 0) ? true : false;
					if(bProcessMeshes == true)
					{
						auto pRenderComponent = const_cast<CBaseEntity*>(info.entity)->GetRenderComponent();
						if(pRenderComponent)
							;//pRenderComponent->UpdateRenderData(drawCmd); // TODO
					}
				}
			}
			//

			drawCmd->RecordBeginRenderPass(*rt,layer);

			// Draw entities
			const auto fDraw = [this,layerFlag,layer,&drawCmd,&csm](pragma::ShaderShadowCSM &shader,bool bTranslucent) -> bool {
				auto bRetTranslucent = false;
				auto bProcessMeshes = false;
				Material *prevMat = nullptr;
				for(auto &info : m_shadowCasters)
				{
					if(info.entity != nullptr)
					{
						bProcessMeshes = ((info.renderFlags &layerFlag) != 0) ? true : false;
						if(bProcessMeshes == true)
							shader.RecordBindEntity(*const_cast<CBaseEntity *>(info.entity), csm.GetStaticPendingViewProjectionMatrix(layer));
					}
					if(info.mesh != nullptr && bProcessMeshes == true)
					{
						if(info.material != nullptr && info.material->IsTranslucent())
						{
							if(bTranslucent == true)
							{
								auto &shaderTranslucent = static_cast<pragma::ShaderShadowCSMTransparent&>(shader);
								if(info.material == prevMat || shaderTranslucent.RecordBindMaterial(static_cast<CMaterial &>(*info.material)) == true)
								{
									shaderTranslucent.RecordDraw(*const_cast<CModelSubMesh *>(info.mesh));
									prevMat = info.material;
								}
							}
							else
								bRetTranslucent = true;
						}
						else if(bTranslucent == false)
							shader.RecordDraw(*const_cast<CModelSubMesh *>(info.mesh));
					}
				}
				return bRetTranslucent;
			};
			auto *scene = c_game->GetRenderScene();
			auto *renderer = scene ? scene->GetRenderer() : nullptr;
			if(renderer && renderer->IsRasterizationRenderer() && shaderCsm.RecordBeginDraw(drawCmd) == true)
			{
				shaderCsm.RecordBindLight(*pLightComponent);
				auto bHasTranslucents = fDraw(shaderCsm,false);

				if(drawParticleShadows == true)
				{
					// TODO: Only culled particles
					EntityIterator entIt {*c_game};
					entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
					for(auto *ent : entIt)
					{
						auto p = ent->GetComponent<pragma::CParticleSystemComponent>();
						if(p.valid() && p->GetCastShadows() == true)
							p->RecordRenderShadow(drawCmd, *scene, *static_cast<pragma::CRasterizationRendererComponent *>(renderer), pLightComponent.get(), layer);
					}
				}

				shaderCsm.RecordEndDraw();
				if(bHasTranslucents == true && shaderCsmTransparent != nullptr)
				{
					if(shaderCsmTransparent->RecordBeginDraw(drawCmd) == true)
					{
						shaderCsmTransparent->RecordBindLight(*pLightComponent);
						fDraw(*shaderCsmTransparent,true); // Draw translucent shadows
						shaderCsmTransparent->RecordEndDraw();
					}
				}
			}

			drawCmd->RecordEndRenderPass();
		}
	}
#endif
}
