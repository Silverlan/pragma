#include "stdafx_client.h"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/model/c_model.h>
#include <pragma/model/c_modelmesh.h>
#include <prosper_command_buffer.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void ShadowRenderer::RenderCSMShadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightDirectionalComponent &light,bool drawParticleShadows)
{
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

	auto frameId = c_engine->GetLastFrameId();
	if(csm.GetLastUpdateFrameId() == frameId)
		return;
	csm.SetLastUpdateFrameId(frameId);

	const std::array<pragma::CLightComponent::ShadowMapType,1> renderPasses = {pragma::CLightComponent::ShadowMapType::Dynamic};
	auto &staticDepthImg = csm.GetDepthTexture(pragma::CLightComponent::ShadowMapType::Static)->GetImage();
	csm.RenderBatch(drawCmd,light);

	// prosper TODO: Remove this block?:
	/*auto w = staticDepthImg->GetWidth();
	auto h = staticDepthImg->GetHeight();
	if(shaderCsm.BeginDraw(drawCmd,w,h) == true)
	{
	shaderCsm.BindLight(*dirLight);
	// Render static entities
	csm.RenderBatch(*dirLight);
	shaderCsm.EndDraw();
	}*/

	// Render dynamic objects
	for(auto rp : renderPasses)
	{
		if(pLightComponent->ShouldUpdateRenderPass(rp) == false)
			continue;
		auto &tex = csm.GetDepthTexture(rp);
		if(tex == nullptr)
			continue;
		auto bDynamic = (rp == pragma::CLightComponent::ShadowMapType::Dynamic) ? true : false;
		if(bDynamic == true)
			;//staticDepthImg->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		else
			;//pLightComponent->SetStaticResolved(true);

		auto &rt = csm.GetRenderTarget(rp);
		auto &renderPass = csm.GetRenderPass(rp);
		auto &img = tex->GetImage();

		auto numLayers = csm.GetLayerCount();
		/*if(rp == CLightBase::RenderPass::Dynamic)
		{
		// Blit static shadow buffer to dynamic shadow buffer (Static shadows have to be rendered first!)
		shadow->GetDepthTexture(CLightBase::RenderPass::Static)->GetImage()->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		img->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		drawCmd->BlitImage(shadow->GetDepthTexture(CLightBase::RenderPass::Static)->GetImage(),img,vk::ImageSubresourceLayers{img->GetAspectFlags(),0,0,numLayers},vk::ImageSubresourceLayers{img->GetAspectFlags(),0,0,numLayers});
		}*/

		auto clearValues = {
			vk::ClearValue{vk::ClearDepthStencilValue{1.f,0}} // Depth Attachment
		};

		m_shadowCasters.clear();

		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
		for(auto *ent : entIt)
		{
			auto pRenderComponent = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
			if(ent->IsInert() == true || static_cast<CBaseEntity*>(ent)->IsInScene(*shadowScene) || pRenderComponent->ShouldDrawShadow(posCam) == false)
				continue;
			auto &mdlComponent = pRenderComponent->GetModelComponent();
			auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
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
		//auto layout = img->GetLayout(); // prosper TODO
		//auto layoutStatic = (staticDepthImg != nullptr) ? staticDepthImg->GetLayout() : vk::ImageLayout::eUndefined; // prosper TODO
		for(auto layer=decltype(numLayers){0};layer<numLayers;++layer)
		{
			//drawCmd->SetImageLayout(img,layout,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,0u,1u,layer,1); // prosper TODO
			//img->SetInternalLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL); // prosper TODO
			prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,layer);

			prosper::util::ClearImageInfo clearImageInfo {};
			clearImageInfo.subresourceRange.baseArrayLayer = layer;
			clearImageInfo.subresourceRange.layerCount = 1u;
			prosper::util::record_clear_image(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,1.f,clearImageInfo); // Clear this layer
			auto &framebuffer = csm.GetFramebuffer(rp,layer);

			prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,layer);

			//drawCmd->SetImageLayout(img,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0u,1u,layer,1); // prosper TODO
			//img->SetInternalLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // prosper TODO
			if(bDynamic == true)
			{
				// Blit static shadow buffer to dynamic shadow buffer (Static shadows have to be rendered first!)
				//drawCmd->SetImageLayout(staticDepthImg,layoutStatic,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,0u,1u,layer,1); // prosper TODO
				//staticDepthImg->SetInternalLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL); // prosper TODO

				//drawCmd->SetImageLayout(img,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,0u,1u,layer,1); // prosper TODO
				//img->SetInternalLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL); // prosper TODO

				prosper::util::record_image_barrier(*(*drawCmd),*(*staticDepthImg),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,layer);
				prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,layer);

				prosper::util::BlitInfo blitInfo {};
				blitInfo.srcSubresourceLayer.base_array_layer = blitInfo.dstSubresourceLayer.base_array_layer = layer;
				blitInfo.srcSubresourceLayer.layer_count = blitInfo.dstSubresourceLayer.layer_count = 1u;
				prosper::util::record_blit_image(*(*drawCmd),blitInfo,*(*staticDepthImg),*(*img));

				prosper::util::record_image_barrier(*(*drawCmd),*(*staticDepthImg),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,layer);
				prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,layer);

				//drawCmd->SetImageLayout(img,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0u,1u,layer,1); // prosper TODO
				//img->SetInternalLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // Barriers need to be inserted again // prosper TODO
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
						if(pRenderComponent.valid())
							pRenderComponent->UpdateRenderData(drawCmd);
					}
				}
			}
			//

			prosper::util::record_begin_render_pass(*(*drawCmd),*rt,layer);

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
							shader.BindEntity(*const_cast<CBaseEntity*>(info.entity),csm.GetStaticPendingViewProjectionMatrix(layer));
					}
					if(info.mesh != nullptr && bProcessMeshes == true)
					{
						if(info.material != nullptr && info.material->IsTranslucent())
						{
							if(bTranslucent == true)
							{
								auto &shaderTranslucent = static_cast<pragma::ShaderShadowCSMTransparent&>(shader);
								if(info.material == prevMat || shaderTranslucent.BindMaterial(static_cast<CMaterial&>(*info.material)) == true)
								{
									shaderTranslucent.Draw(*const_cast<CModelSubMesh*>(info.mesh));
									prevMat = info.material;
								}
							}
							else
								bRetTranslucent = true;
						}
						else if(bTranslucent == false)
							shader.Draw(*const_cast<CModelSubMesh*>(info.mesh));
					}
				}
				return bRetTranslucent;
			};
			auto &scene = c_game->GetRenderScene();
			auto *renderer = scene->GetRenderer();
			if(renderer && renderer->IsRasterizationRenderer() && shaderCsm.BeginDraw(drawCmd) == true)
			{
				shaderCsm.BindLight(*pLightComponent);
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
							p->RenderShadow(drawCmd,*static_cast<pragma::rendering::RasterizationRenderer*>(renderer),pLightComponent.get(),layer);
					}
				}

				shaderCsm.EndDraw();
				if(bHasTranslucents == true && shaderCsmTransparent != nullptr)
				{
					if(shaderCsmTransparent->BeginDraw(drawCmd) == true)
					{
						shaderCsmTransparent->BindLight(*pLightComponent);
						fDraw(*shaderCsmTransparent,true); // Draw translucent shadows
						shaderCsmTransparent->EndDraw();
					}
				}
			}

			prosper::util::record_end_render_pass(*(*drawCmd));

			/*if(type == LightType::Directional && entWorld != nullptr)
			{
			auto *cmdBuffer = static_cast<CLightDirectional*>(light)->GetShadowCommandBuffer(i);
			if(cmdBuffer != nullptr)
			{
			auto *shadowCsm = static_cast<ShadowMapCasc*>(shadow);
			drawCmd->BeginRenderPass(shadowCsm->GetRenderPassKeep(),shadowCsm->GetFramebufferKeep(i),w,h,clearValues,vk::SubpassContents::eSecondaryCommandBuffers);
			if(static_cast<Shader::Base*>(worldShader)->BeginDraw() == true)
			{
			drawCmd->SetViewport(w,h);
			drawCmd->SetScissor(w,h); // Required, but why? We don't have a dynamic scissor state...
			auto idx = (*cmdBuffer)->Swap();
			auto entMvp = static_cast<ShadowMapCasc*>(shadow)->GetViewProjectionMatrix(i) *(*entWorld->GetTransformationMatrix());
			auto &buf = *scene->GetCSMShadowBuffer(i,idx);
			buf->MapMemory(&entMvp,true);

			drawCmd->ExecuteCommand(*(*cmdBuffer)->GetCommandBuffer(idx));
			static_cast<Shader::Base*>(worldShader)->EndDraw();
			}
			drawCmd->EndRenderPass();
			}
			}*/
		}
		//if(bDynamic == true)
		//	pLightComponent->PostRenderShadow();
	}
}
