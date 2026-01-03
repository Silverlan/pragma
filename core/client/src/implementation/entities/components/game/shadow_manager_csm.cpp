// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.game_shadow_manager;
import :engine;
import :entities.components.renderer;
import :game;

using namespace pragma;

void ShadowRenderer::RenderCSMShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, BaseEnvLightDirectionalComponent &light, bool drawParticleShadows)
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
	auto *cam = pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
	auto &posCam = cam ? cam->GetEntity().GetPosition() : uvec::PRM_ORIGIN;
	auto &csm = *hShadowMap;

	auto frameId = pragma::get_cengine()->GetRenderContext().GetLastFrameId();
	if(csm.GetLastUpdateFrameId() == frameId)
		return;
	csm.SetLastUpdateFrameId(frameId);

	const std::array<pragma::rendering::ShadowMapType,1> renderPasses = {pragma::rendering::ShadowMapType::Dynamic};
	auto &staticDepthImg = csm.GetDepthTexture(pragma::rendering::ShadowMapType::Static)->GetImage();
	csm.RenderBatch(drawCmd,light);

	// Render dynamic objects
	for(auto rp : renderPasses)
	{
		if(pLightComponent->ShouldUpdateRenderPass(rp) == false)
			continue;
		auto *tex = csm.GetDepthTexture(rp);
		if(tex == nullptr)
			continue;
		auto bDynamic = (rp == pragma::rendering::ShadowMapType::Dynamic) ? true : false;
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

		pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
		for(auto *ent : entIt)
		{
			auto pRenderComponent = static_cast<pragma::ecs::CBaseEntity*>(ent)->GetRenderComponent();
			if(ent->IsInert() == true || static_cast<pragma::ecs::CBaseEntity*>(ent)->IsInScene(*shadowScene) || pRenderComponent->ShouldDrawShadow(posCam) == false)
				continue;
			auto *mdlComponent = pRenderComponent->GetModelComponent();
			auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
			if(mdl == nullptr)
				continue;
			uint32_t renderFlags = 0;
			if(pLightComponent->ShouldPass(static_cast<pragma::ecs::CBaseEntity&>(*ent),renderFlags) == false)
				continue;
			auto &lodMeshes = pRenderComponent->GetLODMeshes();
			if(lodMeshes.empty())
				continue;

			if(m_shadowCasters.size() == m_shadowCasters.capacity())
				m_shadowCasters.reserve(m_shadowCasters.size() +100);
			m_shadowCasters.push_back({});
			auto &info = m_shadowCasters.back();
			info.entity = static_cast<pragma::ecs::CBaseEntity*>(ent);
			info.renderFlags = renderFlags;

			for(auto &mesh : lodMeshes)
			{
				auto meshRenderFlags = 0u;
				if(pLightComponent->ShouldPass(static_cast<pragma::ecs::CBaseEntity&>(*ent),*static_cast<pragma::geometry::CModelMesh*>(mesh.get()),meshRenderFlags) == false)
					continue;
				for(auto &subMesh : mesh->GetSubMeshes())
				{
					auto matIdx = mdl->GetMaterialIndex(*subMesh);
					auto *mat = matIdx.has_value() ? const_cast<pragma::asset::Model&>(*mdl).GetMaterial(*matIdx) : nullptr;
					if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO
						continue;
					if(m_shadowCasters.size() == m_shadowCasters.capacity())
						m_shadowCasters.reserve(m_shadowCasters.size() +100);
					m_shadowCasters.push_back({});
					auto &info = m_shadowCasters.back();
					info.mesh = static_cast<pragma::geometry::CModelSubMesh*>(subMesh.get());
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
						auto pRenderComponent = const_cast<pragma::ecs::CBaseEntity*>(info.entity)->GetRenderComponent();
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
				material::Material *prevMat = nullptr;
				for(auto &info : m_shadowCasters)
				{
					if(info.entity != nullptr)
					{
						bProcessMeshes = ((info.renderFlags &layerFlag) != 0) ? true : false;
						if(bProcessMeshes == true)
							shader.RecordBindEntity(*const_cast<pragma::ecs::CBaseEntity *>(info.entity), csm.GetStaticPendingViewProjectionMatrix(layer));
					}
					if(info.mesh != nullptr && bProcessMeshes == true)
					{
						if(info.material != nullptr && info.material->IsTranslucent())
						{
							if(bTranslucent == true)
							{
								auto &shaderTranslucent = static_cast<pragma::ShaderShadowCSMTransparent&>(shader);
								if(info.material == prevMat || shaderTranslucent.RecordBindMaterial(static_cast<material::CMaterial &>(*info.material)) == true)
								{
									shaderTranslucent.RecordDraw(*const_cast<pragma::geometry::CModelSubMesh *>(info.mesh));
									prevMat = info.material;
								}
							}
							else
								bRetTranslucent = true;
						}
						else if(bTranslucent == false)
							shader.RecordDraw(*const_cast<pragma::geometry::CModelSubMesh *>(info.mesh));
					}
				}
				return bRetTranslucent;
			};
			auto *scene = pragma::get_cgame()->GetRenderScene<pragma::CSceneComponent>();
			auto *renderer = scene ? scene->GetRenderer<pragma::CRendererComponent>() : nullptr;
			if(renderer && renderer->IsRasterizationRenderer() && shaderCsm.RecordBeginDraw(drawCmd) == true)
			{
				shaderCsm.RecordBindLight(*pLightComponent);
				auto bHasTranslucents = fDraw(shaderCsm,false);

				if(drawParticleShadows == true)
				{
					// TODO: Only culled particles
					pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
					entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::ecs::CParticleSystemComponent>>();
					for(auto *ent : entIt)
					{
						auto p = ent->GetComponent<pragma::ecs::CParticleSystemComponent>();
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
