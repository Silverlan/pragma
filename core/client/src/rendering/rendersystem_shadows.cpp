#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/lighting/c_light.h"
#include "pragma/rendering/lighting/c_light_spot.h"
#include "pragma/rendering/lighting/c_light_point.h"
#include "pragma/rendering/lighting/c_light_directional.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/c_world.h"
#include <pragma/model/model.h>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_iterator.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;
struct ShadowRenderInfo
{
	const CBaseEntity *entity = nullptr;
	const CModelSubMesh *mesh = nullptr;
	uint32_t renderFlags = 0;
	Material *material = nullptr;
};
static std::vector<ShadowRenderInfo> s_shadowRenderMeshInfos {};

static CVar cvParticleQuality = GetClientConVar("cl_render_particle_quality");
static void render_csm_shadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightDirectionalComponent &light,bool bDrawParticleShadows)
{
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
	auto &cam = *c_game->GetRenderCamera();
	auto &posCam = cam.GetPos();
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
					if(mat->GetShaderIdentifier() == "nodraw") // TODO
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
			if(shaderCsm.BeginDraw(drawCmd) == true)
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
							p->RenderShadow(drawCmd,*scene,pLightComponent.get(),layer);
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
}

static void render_shadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightComponent &lightBase)
{
	if(lightBase.GetShadowType() == pragma::BaseEnvLightComponent::ShadowType::None)
		return;
	auto type = LightType::Invalid;
	auto *pLight = lightBase.GetLight(type);
	auto bDrawParticleShadows = (cvParticleQuality->GetInt() >= 3) ? true : false;
	if(type == LightType::Directional)
	{
		render_csm_shadows(drawCmd,static_cast<pragma::CLightDirectionalComponent&>(*pLight),bDrawParticleShadows);
		return;
	}

	auto hShader = c_game->GetGameShader(CGame::GameShader::Shadow);
	auto hShaderTransparent = c_game->GetGameShader(CGame::GameShader::ShadowTransparent);
	auto hShaderSpot = c_game->GetGameShader(CGame::GameShader::ShadowSpot);
	auto hShaderTransparentSpot = c_game->GetGameShader(CGame::GameShader::ShadowTransparentSpot);
	if(hShader.expired() || hShaderSpot.expired())// || hShaderTransparent.expired() || hShaderTransparentSpot.expired()) // prosper TODO: Transparent shadow shaders
		return;
	auto *shadowMap = lightBase.GetShadowMap();
	if(shadowMap == nullptr)
		return;
	auto &shader = (type != LightType::Spot) ? static_cast<pragma::ShaderShadow&>(*hShader.get()) : static_cast<pragma::ShaderShadow&>(*hShaderSpot.get());
	pragma::ShaderShadowTransparent *shaderTransparent = nullptr;
	if(type != LightType::Spot)
		shaderTransparent = static_cast<pragma::ShaderShadowTransparent*>(hShaderTransparent.expired() == false ? hShaderTransparent.get() : nullptr);
	else
		shaderTransparent = static_cast<pragma::ShaderShadowTransparent*>(hShaderTransparentSpot.expired() == false ? hShaderTransparentSpot.get() : nullptr);
#if DEBUG_SHADOWS == 1
	s_shadowMeshCount = 0;
	s_shadowIndexCount = 0;
	s_shadowTriangleCount = 0;
	s_shadowVertexCount = 0;
#endif
	//lightBase->UpdateCulledMeshes();
	if(shadowMap->HasRenderTarget() == true) // If shadow map has render target, it has already been rendered this frame
	{
		lightBase.SetShadowMapIndex(std::numeric_limits<uint32_t>::max());
		return;
	}
	auto wpRt = shadowMap->RequestRenderTarget();
	if(wpRt.expired() == true)
	{
		lightBase.SetShadowMapIndex(std::numeric_limits<uint32_t>::max());
		return;
	}
	auto rt = wpRt.lock();
	lightBase.SetShadowMapIndex(rt->index);
	
	auto &ent = lightBase.GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pRadiusComponent = ent.GetComponent<pragma::CRadiusComponent>();
	if(pTrComponent.expired() || pRadiusComponent.expired())
		return;
	auto &lightPos = pTrComponent->GetPosition();
	auto lightDist = pRadiusComponent->GetRadius();

	const auto fIterateEntityMeshes = [](const Anvil::CommandBufferBase &drawCmd,pragma::CLightComponent &light,const Vector3 &lightPos,float lightDist,
			const std::function<void(const CBaseEntity*,uint32_t)> &entCallback,
			//const std::function<void(const CModelMesh*,uint32_t)> &meshCallback,
			const std::function<void(const Model&,const CModelSubMesh*,uint32_t)> &subMeshCallback
		) {
		auto &octree = c_game->GetScene()->GetOcclusionOctree();
		octree.IterateObjects([&lightPos,&lightDist](const OcclusionOctree<CBaseEntity*>::Node &node) -> bool {
			auto &bounds = node.GetWorldBounds();
			return Intersection::AABBSphere(bounds.first,bounds.second,lightPos,lightDist);
		},[&light,&lightPos,&drawCmd,&entCallback,&subMeshCallback](const CBaseEntity *ent) {
			auto pRenderComponent = ent->GetRenderComponent();
			if(pRenderComponent.expired() || pRenderComponent->ShouldDrawShadow(lightPos) == false || ent->IsWorld() == true)
				return;
			uint32_t renderFlags = 0;
			if(light.ShouldPass(*ent,renderFlags) == false)
				return;
			entCallback(ent,renderFlags);
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
						subMeshCallback(*mdl,static_cast<CModelSubMesh*>(subMesh.get()),renderFlags);
					}
				}
			}
		});
	
		// Render world geometry
		auto *pWorld = c_game->GetWorld();
		if(pWorld != nullptr)
		{
			auto &entWorld = static_cast<CBaseEntity&>(pWorld->GetEntity());
			auto mdlComponent = entWorld.GetModelComponent();
			auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
			if(mdl != nullptr)
			{
				auto meshTree = static_cast<pragma::CWorldComponent*>(pWorld)->GetMeshTree();
				if(meshTree != nullptr)
				{
					uint32_t renderFlags = 0;
					if(light.ShouldPass(entWorld,renderFlags) == true)
					{
						entCallback(&entWorld,renderFlags);
						meshTree->IterateObjects([&lightPos,&lightDist](const OcclusionOctree<std::shared_ptr<ModelMesh>>::Node &node) -> bool {
							auto &bounds = node.GetWorldBounds();
							return Intersection::AABBSphere(bounds.first,bounds.second,lightPos,lightDist);
						},[&entWorld,&mdl,&lightPos,&lightDist,&light,&drawCmd,&subMeshCallback](const std::shared_ptr<ModelMesh> &mesh) {
							uint32_t renderFlags = 0;
							if(light.ShouldPass(entWorld,*static_cast<CModelMesh*>(mesh.get()),renderFlags) == false)
								return;
							//meshCallback(static_cast<CModelMesh*>(mesh.get()),renderFlags);
							for(auto &subMesh : mesh->GetSubMeshes())
							{
								if(light.ShouldPass(*mdl,*static_cast<CModelSubMesh*>(subMesh.get())) == false)
									continue;
								auto *mat = mdl->GetMaterial(subMesh->GetTexture());
								if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO
									continue;
								subMeshCallback(*mdl,static_cast<CModelSubMesh*>(subMesh.get()),renderFlags);
							}
						});
					}
				}
			}
		}
	};

	auto &smRt = shadowMap->GetDepthRenderTarget();
	auto &tex = smRt->GetTexture();
	if(tex != nullptr)
	{
		auto &renderPass = shadowMap->GetRenderPass();
		auto &img = tex->GetImage();
		s_shadowRenderMeshInfos.clear();

		// We need to iterate all entities BEFORE the render pass has started, because the entity render buffers have to be updated
		// outside of an active render pass
		auto bSpot = type == LightType::Spot;
		fIterateEntityMeshes(*(*drawCmd),lightBase,lightPos,lightDist,[bSpot,&drawCmd](const CBaseEntity *ent,uint32_t renderFlags) {
			if(s_shadowRenderMeshInfos.size() == s_shadowRenderMeshInfos.capacity())
				s_shadowRenderMeshInfos.reserve(s_shadowRenderMeshInfos.size() +100);
			s_shadowRenderMeshInfos.push_back({});
			auto &info = s_shadowRenderMeshInfos.back();
			info.entity = ent;
			info.renderFlags = (bSpot == false) ? renderFlags : 1u; // Spot-lights only have 1 layer; Always render
			auto pRenderComponent = ent->GetRenderComponent();
			if(pRenderComponent.valid())
				pRenderComponent->UpdateRenderData(drawCmd);
		},[](const Model &mdl,const CModelSubMesh *subMesh,uint32_t renderFlags) {
			auto *mat = const_cast<Model&>(mdl).GetMaterial(subMesh->GetTexture());
			if(s_shadowRenderMeshInfos.size() == s_shadowRenderMeshInfos.capacity())
				s_shadowRenderMeshInfos.reserve(s_shadowRenderMeshInfos.size() +100);
			s_shadowRenderMeshInfos.push_back({});
			auto &info = s_shadowRenderMeshInfos.back();
			info.mesh = subMesh;
			info.renderFlags = renderFlags;
			info.material = mat;
		});

		auto numLayers = shadowMap->GetLayerCount();

		// Clear all layers
		prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		prosper::util::record_clear_image(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,1.f);
		prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		//auto layout = img->GetLayout(); // prosper TODO
		for(auto layerId=decltype(numLayers){0};layerId<numLayers;++layerId)
		{
			auto &matLight = lightBase.GetTransformationMatrix(layerId);

			/*EntityIterator entIt{*c_game};
			entIt.AttachFilter<EntityIteratorFilterClass>("env_light_spot");
			auto it = entIt.begin();
			if(it != entIt.end())
			{
				auto *ent = *it;
				auto pLightSpot = ent->GetComponent<pragma::CLightSpotComponent>();
				auto pTrComponent = ent->GetTransformComponent();
				auto pos = pTrComponent->GetPosition();
				auto dir = pTrComponent->GetForward();
				auto outerCutoffAngle = pLightSpot->GetOuterCutoffAngle();
				auto pRadiusComponent = ent->GetComponent<pragma::CRadiusComponent>();
				auto radius = pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f;
				auto vm = glm::lookAtRH(pos,pos +dir,uvec::get_perpendicular(dir));
				auto pm = glm::perspectiveRH<float>(CFloat(umath::deg_to_rad(outerCutoffAngle *2.f)),1.f,2.f,radius);
				auto scale = Vector3(-1.f,-1.f,1.f);
				pm = glm::scale(pm,scale);
				matLight = pm *vm;

				auto pLightComponent = ent->GetComponent<pragma::CLightComponent>();
				auto &shadowBuffer = pLightComponent->GetShadowBuffer();
				auto mBiasTransform = umat::BIAS *matLight;
				std::array<Mat4,3> matrices = {mBiasTransform,vm,pm};//pLightSpot->GetViewMatrix(),pLightSpot->GetProjectionMatrix()};
				c_engine->ScheduleRecordUpdateBuffer(shadowBuffer,0ull,matrices);
				//	SetViewMatrix();
	//auto p = 
				//matLight = ent->GetComponent<pragma::CLightPointComponent>()->GetTransformationMatrix();
			}*/

			//drawCmd->SetImageLayout(img,layout,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,0u,1u,layerId,1); // prosper TODO
			//img->SetInternalLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL); // prosper TODO

			//prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,layerId);
			/*prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,layerId);

			prosper::util::ClearImageInfo clearImageInfo {};
			clearImageInfo.subresourceRange.baseArrayLayer = layerId;
			clearImageInfo.subresourceRange.layerCount = 1u;
			if(skip == false)
				prosper::util::record_clear_image(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,1.f,clearImageInfo); // Clear this layer

			prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,layerId);
			*/
			auto &framebuffer = shadowMap->GetFramebuffer(layerId);

			//drawCmd->SetImageLayout(img,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0u,1u,layerId,1); // prosper TODO
			//img->SetInternalLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // prosper TODO

			vk::ClearValue clearVal {vk::ClearDepthStencilValue{1.f}};
			if(prosper::util::record_begin_render_pass(*(*drawCmd),*smRt,layerId,&clearVal) == true)
			{
				//prosper::util::record_clear_attachment(*(*drawCmd),*(*img),1.f,layerId);
				// Render entities
				const auto fDraw = [&lightBase,&drawCmd,&lightPos,&lightDist,&matLight,&fIterateEntityMeshes,&layerId](pragma::ShaderShadow &shader,bool bTranslucent) -> bool {
					auto bRetTranslucent = false;
					if(shader.BeginDraw(drawCmd) == true)
					{
						if(shader.BindLight(lightBase) == true)
						{
							// prosper TODO: Deprecated?
							/*if(bPointLight == false && bTranslucent == false)
							{
								fIterateEntityMeshes(*drawCmd,light,lightPos,lightDist,[&shader,&drawCmd,&matLight](const CBaseEntity *ent,uint32_t renderFlags) {
									shader.BindEntity(*const_cast<CBaseEntity*>(ent),matLight);

									// Only needed if there are translucent meshes
									if(s_shadowRenderMeshInfos.size() == s_shadowRenderMeshInfos.capacity())
										s_shadowRenderMeshInfos.reserve(s_shadowRenderMeshInfos.size() +100);
									s_shadowRenderMeshInfos.push_back({});
									auto &info = s_shadowRenderMeshInfos.back();
									info.entity = ent;
									info.renderFlags = 1u; // Spot-lights only have 1 layer; Always render
								},[&shader,&drawCmd,&bRetTranslucent](const Model &mdl,const CModelSubMesh *mesh,uint32_t renderFlags) mutable {
									auto *mat = const_cast<Model&>(mdl).GetMaterial(mesh->GetTexture());
									if(mat == nullptr || mat->IsTranslucent() == false)
										shader.Draw(*const_cast<CModelSubMesh*>(mesh));
									else
									{
										bRetTranslucent = true;
										if(s_shadowRenderMeshInfos.size() == s_shadowRenderMeshInfos.capacity())
											s_shadowRenderMeshInfos.reserve(s_shadowRenderMeshInfos.size() +100);
										s_shadowRenderMeshInfos.push_back({});
										auto &info = s_shadowRenderMeshInfos.back();
										info.mesh = mesh;
										info.renderFlags = renderFlags;
										info.material = mat;
									}
								});
							}
							else*/
							{
								auto layerFlag = 1<<layerId;
								auto bProcessMeshes = false;
								Material *prevMat = nullptr;
								for(auto &info : s_shadowRenderMeshInfos)
								{
									if(info.entity != nullptr)
									{
										bProcessMeshes = ((info.renderFlags &layerFlag) != 0) ? true : false;
										if(bProcessMeshes == true)
											shader.BindEntity(*const_cast<CBaseEntity*>(info.entity),matLight);
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
												bRetTranslucent = true;
										}
										else if(bTranslucent == false)
											shader.Draw(*const_cast<CModelSubMesh*>(info.mesh));
									}
								}
							}
						}
						shader.EndDraw();
					}
					return bRetTranslucent;
				};

				auto bHasTranslucents = fDraw(shader,false);
				if(bHasTranslucents == true && shaderTransparent != nullptr)
					fDraw(*shaderTransparent,true); // Draw translucent shadows
				if(bDrawParticleShadows == true)
				{
					auto &scene = c_game->GetRenderScene();
					// TODO: Only culled particles
					EntityIterator entIt {*c_game};
					entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
					for(auto *ent : entIt)
					{
						auto p = ent->GetComponent<pragma::CParticleSystemComponent>();
						if(p.valid() && p->GetCastShadows() == true)
							p->RenderShadow(drawCmd,*scene,&lightBase,layerId);
					}
				}
				prosper::util::record_end_render_pass(*(*drawCmd));
			}
		}
	}
#if 0
	auto bDrawParticleShadows = (cvParticleQuality->GetInt() >= 3) ? true : false;
	const std::array<CLightBase::RenderPass,2> renderPasses = {CLightBase::RenderPass::Static,CLightBase::RenderPass::Dynamic};
	auto &staticDepthImg = shadow->GetDepthTexture(CLightBase::RenderPass::Static)->GetImage();
	for(auto rp : renderPasses)
	{
		if(light->ShouldUpdateRenderPass(rp) == false)
			continue;
		auto &tex = shadow->GetDepthTexture(rp);
		if(!tex.IsValid())
			continue;
		auto bDynamic = (rp == CLightBase::RenderPass::Dynamic) ? true : false;
		if(bDynamic == true)
			staticDepthImg->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		else
		{
			//Con::cerr<<"Updating static geometry..."<<Con::endl;
			light->SetStaticResolved(true);
		}
		auto &context = c_engine->GetRenderContext();
		auto &drawCmd = context.GetDrawCmd();
		auto &renderPass = shadow->GetRenderPass(rp);
		auto &img = tex->GetImage();
		//auto &scene = c_game->GetRenderScene();
		//auto *entWorld = c_game->GetWorld();

		auto numLayers = shadow->GetLayerCount();
		/*if(rp == CLightBase::RenderPass::Dynamic)
		{
			// Blit static shadow buffer to dynamic shadow buffer (Static shadows have to be rendered first!)
			shadow->GetDepthTexture(CLightBase::RenderPass::Static)->GetImage()->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
			img->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
			drawCmd->BlitImage(shadow->GetDepthTexture(CLightBase::RenderPass::Static)->GetImage(),img,vk::ImageSubresourceLayers{img->GetAspectFlags(),0,0,numLayers},vk::ImageSubresourceLayers{img->GetAspectFlags(),0,0,numLayers});
		}*/

		auto &particles = c_game->GetParticleSystems();
		auto w = img->GetWidth();
		auto h = img->GetHeight();
		auto clearValues = {
			vk::ClearValue{vk::ClearDepthStencilValue{1.f,0}} // Depth Attachment
		};
		auto type = light->GetType();
		Shader::Shadow *worldShader = (type != LightType::Directional) ? &shader : &shaderCsm;
		for(auto i=decltype(numLayers){0};i<numLayers;++i)
		{
			if(shadow->ShouldUpdateLayer(i) == false)
				continue;
			img->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,0,img->GetMipmapLevels(),i,1); // Required for clearing the image
			drawCmd->ClearImage(img,1.f,0,img->GetMipmapLevels(),i,1); // Clear this layer
			auto &framebuffer = shadow->GetFramebuffer(rp,i);

			img->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0,img->GetMipmapLevels(),i,1);
			drawCmd->BeginRenderPass(renderPass,framebuffer,w,h);
			if(bDynamic == true)
			{
				// Blit static shadow buffer to dynamic shadow buffer (Static shadows have to be rendered first!)
				drawCmd->BlitImage(staticDepthImg,img,vk::ImageSubresourceLayers{staticDepthImg->GetAspectFlags(),0,i,1},vk::ImageSubresourceLayers{img->GetAspectFlags(),0,i,1});
				img->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0,img->GetMipmapLevels(),i,1); // Barriers need to be inserted again
			}
			worldShader->Render(drawCmd,light,i,rp);
			if(bDrawParticleShadows == true)
			{
				for(auto *p : particles)
				{
					if(p != nullptr && p->GetCastShadows() == true)
						p->RenderShadow(light,i);
				}
			}
		
			drawCmd->EndRenderPass();

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
			light->PostRenderShadow();
	}
#endif
#if DEBUG_SHADOWS == 1
	Con::cerr<<"-----------------------------"<<Con::endl;
	Con::cout<<"Shadow Mesh Count: "<<s_shadowMeshCount<<Con::endl;
	Con::cout<<"Shadow Index Count: "<<s_shadowIndexCount<<Con::endl;
	Con::cout<<"Shadow Triangle Count: "<<s_shadowTriangleCount<<Con::endl;
	Con::cout<<"Shadow Vertex Count: "<<s_shadowVertexCount<<Con::endl;
#endif
}

void RenderSystem::RenderShadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::vector<pragma::CLightComponent*> &lights)
{
	if(lights.empty())
		return;

	// Directional light source is always first in array, but should be rendered last, so we iterate backwards
	// Otherwise the shadowmaps for all light sources coming after are cleared; TODO: Find out why (Something to do with secondary command buffer, see CLightDirectional::GetShadowCommandBuffer / Shadow Shader)
	auto frameId = c_engine->GetLastFrameId();
	for(auto it=lights.rbegin();it!=lights.rend();++it)
	{
		auto *light = *it;
		auto tLast = light->GetLastTimeShadowRendered();
		if(tLast == frameId) // Don't render shadows if they have already been rendered during this frame
			continue;
		light->SetLastTimeShadowRendered(frameId);
		render_shadows(drawCmd,*light);
	}
}
