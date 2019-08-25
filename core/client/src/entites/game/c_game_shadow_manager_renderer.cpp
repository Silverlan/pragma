#include "stdafx_client.h"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

#pragma optimize("",off)
static void render_csm_shadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightDirectionalComponent &light,bool bDrawParticleShadows)
{
#if 0
	auto pLightComponent = light.GetEntity().GetComponent<pragma::CLightComponent>();
	auto hShaderCsm = c_game->GetGameShader(CGame::GameShader::ShadowCSM);
	auto hShaderCsmTransparent = c_game->GetGameShader(CGame::GameShader::ShadowCSMTransparent);
	if(pLightComponent.expired() || hShaderCsm.expired())// || hShaderCsmTransparent.expired()) // prosper TODO: Transparent shadow shader
		return;
	auto *pShadowMap = pLightComponent->GetShadowMap();
	if(pShadowMap == nullptr || pShadowMap->GetType() != ShadowMap::Type::Cascaded)
		return;
	auto &shaderCsm = static_cast<pragma::ShaderShadowCSM&>(*hShaderCsm.get());
	auto *shaderCsmTransparent = static_cast<pragma::ShaderShadowCSMTransparent*>(hShaderCsmTransparent.expired() == false ? hShaderCsmTransparent.get() : nullptr);
	auto *cam = c_game->GetRenderCamera();
	auto &posCam = cam ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	auto &csm = static_cast<ShadowMapCasc&>(*pShadowMap);

	auto frameId = c_engine->GetLastFrameId();
	if(csm.GetLastUpdateFrameId() == frameId)
		return;
	csm.SetLastUpdateFrameId(frameId);

	const std::array<pragma::CLightComponent::RenderPass,1> renderPasses = {pragma::CLightComponent::RenderPass::Dynamic};
	auto &staticDepthImg = csm.GetDepthTexture(pragma::CLightComponent::RenderPass::Static)->GetImage();
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
		auto bDynamic = (rp == pragma::CLightComponent::RenderPass::Dynamic) ? true : false;
		if(bDynamic == true)
			;//staticDepthImg->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		else
			pLightComponent->SetStaticResolved(true);

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

		s_shadowRenderMeshInfos.clear();

		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
		for(auto *ent : entIt)
		{
			auto pRenderComponent = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
			if(ent->IsInert() == true || pRenderComponent->ShouldDrawShadow(posCam) == false)
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

			if(s_shadowRenderMeshInfos.size() == s_shadowRenderMeshInfos.capacity())
				s_shadowRenderMeshInfos.reserve(s_shadowRenderMeshInfos.size() +100);
			s_shadowRenderMeshInfos.push_back({});
			auto &info = s_shadowRenderMeshInfos.back();
			info.entity = static_cast<CBaseEntity*>(ent);
			info.renderFlags = renderFlags;

			for(auto &mesh : lodMeshes)
			{
				auto meshRenderFlags = 0u;
				if(pLightComponent->ShouldPass(static_cast<CBaseEntity&>(*ent),*static_cast<CModelMesh*>(mesh.get()),meshRenderFlags) == false)
					continue;
				for(auto &subMesh : mesh->GetSubMeshes())
				{
					auto *mat = const_cast<Model&>(*mdl).GetMaterial(subMesh->GetTexture());
					if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO
						continue;
					if(s_shadowRenderMeshInfos.size() == s_shadowRenderMeshInfos.capacity())
						s_shadowRenderMeshInfos.reserve(s_shadowRenderMeshInfos.size() +100);
					s_shadowRenderMeshInfos.push_back({});
					auto &info = s_shadowRenderMeshInfos.back();
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
			for(auto &info : s_shadowRenderMeshInfos)
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
			const auto fDraw = [layerFlag,layer,&drawCmd,&csm](pragma::ShaderShadowCSM &shader,bool bTranslucent) -> bool {
				auto bRetTranslucent = false;
				auto bProcessMeshes = false;
				Material *prevMat = nullptr;
				for(auto &info : s_shadowRenderMeshInfos)
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

				if(bDrawParticleShadows == true)
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
		if(bDynamic == true)
			pLightComponent->PostRenderShadow();
	}
#endif
}

ShadowRenderer::ShadowRenderer()
{
	m_shader = c_game->GetGameShader(CGame::GameShader::Shadow);
	m_shaderTransparent = c_game->GetGameShader(CGame::GameShader::ShadowTransparent);
	m_shaderSpot = c_game->GetGameShader(CGame::GameShader::ShadowSpot);
	m_shaderSpotTransparent = c_game->GetGameShader(CGame::GameShader::ShadowTransparentSpot);

	m_octreeCallbacks.nodeCallback = [this](const OcclusionOctree<std::shared_ptr<ModelMesh>>::Node &node) -> bool {
		auto &bounds = node.GetWorldBounds();
		return Intersection::AABBSphere(bounds.first,bounds.second,m_lightSourceData.position,m_lightSourceData.radius);
	};

	m_octreeCallbacks.entityCallback = [this](const CBaseEntity &ent,uint32_t renderFlags) {
		m_currentEntity = &ent;
		m_currentModel = ent.GetModel().get();
		m_currentRenderFlags = renderFlags;
		if(m_currentModel == nullptr)
		{
			m_currentEntity = nullptr;
			return;
		}
		m_shadowCasters.push_back({});
		auto &info = m_shadowCasters.back();
		info.entity = &ent;
		info.renderFlags = (m_lightSourceData.type == LightType::Point) ? renderFlags : 1u; // Spot-lights only have 1 layer, so we can ignore the flags
		auto pRenderComponent = ent.GetRenderComponent();
		if(pRenderComponent.valid())
		{
			// Make sure entity buffer data is up to date
			pRenderComponent->UpdateRenderData(m_lightSourceData.drawCmd);
		}
	};

	m_octreeCallbacks.meshCallback = [this](const std::shared_ptr<ModelMesh> &mesh) {
		auto *ent = m_currentEntity;
		if(m_lightSourceData.light->ShouldPass(*ent,*static_cast<CModelMesh*>(mesh.get()),m_currentRenderFlags) == false)
			return;
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			if(m_lightSourceData.light->ShouldPass(*m_currentModel,*static_cast<CModelSubMesh*>(subMesh.get())) == false)
				continue;
			auto *mat = m_currentModel->GetMaterial(subMesh->GetTexture());
			if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO: Do this properly
				continue;
			m_octreeCallbacks.subMeshCallback(*m_currentModel,*static_cast<CModelSubMesh*>(subMesh.get()),m_currentRenderFlags);
		}
	};

	m_octreeCallbacks.subMeshCallback = [this](const Model &mdl,const CModelSubMesh &subMesh,uint32_t renderFlags) {
		auto *mat = const_cast<Model&>(mdl).GetMaterial(subMesh.GetTexture());
		m_shadowCasters.push_back({});
		auto &info = m_shadowCasters.back();
		info.mesh = &subMesh;
		info.renderFlags = renderFlags;
		info.material = mat;
	};
}

void ShadowRenderer::UpdateWorldShadowCasters(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightComponent &light)
{
	auto *pWorld = c_game->GetWorld();
	if(pWorld == nullptr)
		return;
	auto &entWorld = static_cast<CBaseEntity&>(pWorld->GetEntity());
	auto mdlComponent = entWorld.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	auto meshTree = mdl ? static_cast<pragma::CWorldComponent*>(pWorld)->GetMeshTree() : nullptr;
	if(meshTree == nullptr)
		return;

	uint32_t renderFlags = 0u;
	if(light.ShouldPass(entWorld,renderFlags) == false)
		return;

	// Iterate all meshes in the world and populate m_shadowCasters
	m_octreeCallbacks.entityCallback(entWorld,renderFlags);
	meshTree->IterateObjects(m_octreeCallbacks.nodeCallback,m_octreeCallbacks.meshCallback);
}
void ShadowRenderer::UpdateEntityShadowCasters(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightComponent &light)
{
	auto &scene = c_game->GetScene();
	auto &octree = scene->GetOcclusionOctree();
	// Iterate all entities in the scene and populate m_shadowCasters
	octree.IterateObjects([this](const OcclusionOctree<CBaseEntity*>::Node &node) -> bool {
		auto &bounds = node.GetWorldBounds();
		return Intersection::AABBSphere(bounds.first,bounds.second,m_lightSourceData.position,m_lightSourceData.radius);
		},[this,&light,&drawCmd](const CBaseEntity *ent) {
			auto pRenderComponent = ent->GetRenderComponent();
			if(pRenderComponent.expired() || pRenderComponent->ShouldDrawShadow(m_lightSourceData.position) == false || ent->IsWorld() == true)
				return;
			uint32_t renderFlags = 0;
			if(light.ShouldPass(*ent,renderFlags) == false)
				return;
			m_octreeCallbacks.entityCallback(*ent,renderFlags);
			auto &mdlComponent = pRenderComponent->GetModelComponent();
			if(mdlComponent.valid())
			{
				auto mdl = mdlComponent->GetModel();
				for(auto &mesh : static_cast<pragma::CModelComponent&>(*mdlComponent).GetLODMeshes())
				{
					if(light.ShouldPass(*ent,*static_cast<CModelMesh*>(mesh.get()),renderFlags) == false)
						continue;
					//meshCallback(static_cast<CModelMesh*>(mesh.get()),renderFlags);
					for(auto &subMesh : mesh->GetSubMeshes())
					{
						if(light.ShouldPass(*mdl,*static_cast<CModelSubMesh*>(subMesh.get())) == false)
							continue;
						auto *mat = mdl->GetMaterial(subMesh->GetTexture());
						if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO
							continue;
						m_octreeCallbacks.subMeshCallback(*mdl,*static_cast<CModelSubMesh*>(subMesh.get()),renderFlags);
					}
				}
			}
		});
}

bool ShadowRenderer::UpdateShadowCasters(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightComponent &light,pragma::CLightComponent::ShadowMapType smType)
{
	m_shadowCasters.clear();
	auto hShadowMap = light.GetShadowMap(smType);
	if(hShadowMap.expired())
		return false;
	auto frameId = c_engine->GetLastFrameId();
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
	return true;
}

ShadowRenderer::RenderResultFlags ShadowRenderer::RenderShadows(
	std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,
	pragma::CLightComponent &light,uint32_t layerId,const Mat4 &depthMVP,
	pragma::ShaderShadow &shader,bool bTranslucent
)
{
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
}

static CVar cvParticleQuality = GetClientConVar("cl_render_particle_quality");
void ShadowRenderer::RenderShadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightComponent &light,pragma::CLightComponent::ShadowMapType smType)
{
	auto hShadowMap = light.GetShadowMap(smType);
	if(hShadowMap.expired() || light.GetShadowType() == pragma::BaseEnvLightComponent::ShadowType::None)
		return;
	auto type = LightType::Invalid;
	auto *pLight = light.GetLight(type);
	auto bDrawParticleShadows = (cvParticleQuality->GetInt() >= 3) ? true : false;
	if(type == LightType::Directional)
	{
		render_csm_shadows(drawCmd,static_cast<pragma::CLightDirectionalComponent&>(*pLight),bDrawParticleShadows);
		return;
	}

	if(UpdateShadowCasters(drawCmd,light,smType) == false)
		return;

	auto &shader = (type != LightType::Spot) ? static_cast<pragma::ShaderShadow&>(*m_shader.get()) : static_cast<pragma::ShaderShadow&>(*m_shaderSpot.get());
	pragma::ShaderShadowTransparent *shaderTransparent = nullptr;
	if(type != LightType::Spot)
		shaderTransparent = static_cast<pragma::ShaderShadowTransparent*>(m_shaderTransparent.expired() == false ? m_shaderTransparent.get() : nullptr);
	else
		shaderTransparent = static_cast<pragma::ShaderShadowTransparent*>(m_shaderSpotTransparent.expired() == false ? m_shaderSpotTransparent.get() : nullptr);
#if DEBUG_SHADOWS == 1
	s_shadowMeshCount = 0;
	s_shadowIndexCount = 0;
	s_shadowTriangleCount = 0;
	s_shadowVertexCount = 0;
#endif

	auto &smRt = hShadowMap->GetDepthRenderTarget();
	auto &tex = smRt->GetTexture();
	if(tex == nullptr)
		return;
	auto &scene = c_game->GetScene();
	auto *renderer = scene->GetRenderer();
	if(renderer->IsRasterizationRenderer() == false)
		renderer = nullptr;

	auto &img = tex->GetImage();
	auto numLayers = hShadowMap->GetLayerCount();
	prosper::util::record_image_barrier(**drawCmd,**img,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	for(auto layerId=decltype(numLayers){0};layerId<numLayers;++layerId)
	{
		auto &depthMVP = light.GetTransformationMatrix(layerId);
		auto &framebuffer = hShadowMap->GetFramebuffer(layerId);

		const vk::ClearValue clearVal {vk::ClearDepthStencilValue{1.f}};
		if(prosper::util::record_begin_render_pass(*(*drawCmd),*smRt,layerId,&clearVal) == false)
			continue;
		auto renderResultFlags = RenderShadows(drawCmd,light,layerId,depthMVP,shader,false);
		if(umath::is_flag_set(renderResultFlags,RenderResultFlags::TranslucentPending) && shaderTransparent != nullptr)
			RenderShadows(drawCmd,light,layerId,depthMVP,shader,true); // Draw translucent shadows
		if(bDrawParticleShadows == true && renderer)
		{
			auto &scene = c_game->GetRenderScene();
			// TODO: Only culled particles
			EntityIterator entIt {*c_game};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
			for(auto *ent : entIt)
			{
				auto p = ent->GetComponent<pragma::CParticleSystemComponent>();
				if(p.valid() && p->GetCastShadows() == true)
					p->RenderShadow(drawCmd,*static_cast<pragma::rendering::RasterizationRenderer*>(renderer),&light,layerId);
			}
		}
		prosper::util::record_end_render_pass(*(*drawCmd));

		prosper::util::ImageSubresourceRange range {layerId};
		prosper::util::record_post_render_pass_image_barrier(
			**drawCmd,**img,
			Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
			range
		);
	}
}

void ShadowRenderer::RenderShadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightComponent &light)
{
	if(m_shader.expired() || m_shaderSpot.expired())
		return;
	// TODO: Only render if update is required
	RenderShadows(drawCmd,light,pragma::CLightComponent::ShadowMapType::Static);
	RenderShadows(drawCmd,light,pragma::CLightComponent::ShadowMapType::Dynamic);
}
#pragma optimize("",on)
