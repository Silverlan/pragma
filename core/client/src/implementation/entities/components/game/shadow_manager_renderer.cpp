// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.game_shadow_manager;
import :engine;
import :game;

using namespace pragma;

ShadowRenderer::ShadowRenderer()
{
	m_shader = get_cgame()->GetGameShader(CGame::GameShader::Shadow);
	m_shaderTransparent = get_cgame()->GetGameShader(CGame::GameShader::ShadowTransparent);
	m_shaderSpot = get_cgame()->GetGameShader(CGame::GameShader::ShadowSpot);
	m_shaderSpotTransparent = get_cgame()->GetGameShader(CGame::GameShader::ShadowTransparentSpot);
	m_shaderCSM = get_cgame()->GetGameShader(CGame::GameShader::ShadowCSM);
	m_shaderCSMTransparent = get_cgame()->GetGameShader(CGame::GameShader::ShadowCSMTransparent);

	m_octreeCallbacks.nodeCallback = [this](const OcclusionOctree<std::shared_ptr<geometry::ModelMesh>>::Node &node) -> bool {
		auto &bounds = node.GetWorldBounds();
		return math::intersection::aabb_sphere(bounds.first, bounds.second, m_lightSourceData.position, m_lightSourceData.radius);
	};

	m_octreeCallbacks.entityCallback = [this](const ecs::CBaseEntity &ent, uint32_t renderFlags) {
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
		info.renderFlags = (m_lightSourceData.type == LightType::Point) ? renderFlags : 1u; // Spot-lights only have 1 layer, so we can ignore the flags
		auto pRenderComponent = ent.GetRenderComponent();
		if(pRenderComponent) {
			// Make sure entity buffer data is up to date
			// TODO
			//pRenderComponent->UpdateRenderData(m_lightSourceData.drawCmd,{}); // TODO: Camera position
		}
	};

	m_octreeCallbacks.meshCallback = [this](const std::shared_ptr<geometry::ModelMesh> &mesh) {
		auto *ent = m_currentEntity;
		if(m_lightSourceData.light->ShouldPass(*ent, *static_cast<geometry::CModelMesh *>(mesh.get()), m_currentRenderFlags) == false)
			return;
		for(auto &subMesh : mesh->GetSubMeshes()) {
			if(m_lightSourceData.light->ShouldPass(*m_currentModel, *static_cast<geometry::CModelSubMesh *>(subMesh.get())) == false)
				continue;
			auto matIdx = m_currentModel->GetMaterialIndex(*subMesh);
			auto *mat = matIdx.has_value() ? m_currentModel->GetMaterial(*matIdx) : nullptr;
			if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO: Do this properly
				continue;
			m_octreeCallbacks.subMeshCallback(*m_currentModel, *static_cast<geometry::CModelSubMesh *>(subMesh.get()), m_currentRenderFlags);
		}
	};

	m_octreeCallbacks.subMeshCallback = [this](const asset::Model &mdl, const geometry::CModelSubMesh &subMesh, uint32_t renderFlags) {
		auto matIdx = mdl.GetMaterialIndex(subMesh);
		auto *mat = matIdx.has_value() ? const_cast<asset::Model &>(mdl).GetMaterial(*matIdx) : nullptr;
		m_shadowCasters.push_back({});
		auto &info = m_shadowCasters.back();
		info.mesh = &subMesh;
		info.renderFlags = renderFlags;
		info.material = mat;
	};
}

void ShadowRenderer::UpdateWorldShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light)
{
	auto *scene = light.FindShadowScene();
	if(scene == nullptr)
		return;
	auto *pWorld = get_cgame()->GetWorld();
	if(pWorld == nullptr)
		return;
	auto &entWorld = static_cast<ecs::CBaseEntity &>(pWorld->GetEntity());
	if(entWorld.IsInScene(*scene) == false)
		return;
	auto &mdl = entWorld.GetModel();
	auto meshTree = mdl ? static_cast<CWorldComponent *>(pWorld)->GetMeshTree() : nullptr;
	if(meshTree == nullptr)
		return;

	uint32_t renderFlags = 0u;
	if(light.ShouldPass(entWorld, renderFlags) == false)
		return;

	// Iterate all meshes in the world and populate m_shadowCasters
	m_octreeCallbacks.entityCallback(entWorld, renderFlags);
	meshTree->IterateObjects(m_octreeCallbacks.nodeCallback, m_octreeCallbacks.meshCallback);
}
void ShadowRenderer::UpdateEntityShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light)
{
	auto *scene = light.FindShadowScene();
	if(scene == nullptr)
		return;
	auto *culler = scene->FindOcclusionCuller<COcclusionCullerComponent>();
	if(culler == nullptr)
		return;
	auto &octree = culler->GetOcclusionOctree();
	// Iterate all entities in the scene and populate m_shadowCasters
	octree.IterateObjects(
	  [this](const OcclusionOctree<ecs::CBaseEntity *>::Node &node) -> bool {
		  auto &bounds = node.GetWorldBounds();
		  return math::intersection::aabb_sphere(bounds.first, bounds.second, m_lightSourceData.position, m_lightSourceData.radius);
	  },
	  [this, &light, &drawCmd, scene](const ecs::CBaseEntity *ent) {
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
			  for(auto &mesh : static_cast<CModelComponent &>(*mdlComponent).GetLODMeshes()) {
				  if(light.ShouldPass(*ent, *static_cast<geometry::CModelMesh *>(mesh.get()), renderFlags) == false)
					  continue;
				  //meshCallback(static_cast<pragma::geometry::CModelMesh*>(mesh.get()),renderFlags);
				  for(auto &subMesh : mesh->GetSubMeshes()) {
					  if(light.ShouldPass(*mdl, *static_cast<geometry::CModelSubMesh *>(subMesh.get())) == false)
						  continue;
					  auto matIdx = mdl->GetMaterialIndex(*subMesh);
					  auto *mat = matIdx.has_value() ? mdl->GetMaterial(*matIdx) : nullptr;
					  if(mat == nullptr || mat->GetShaderIdentifier() == "nodraw") // TODO
						  continue;
					  m_octreeCallbacks.subMeshCallback(*mdl, *static_cast<geometry::CModelSubMesh *>(subMesh.get()), renderFlags);
				  }
			  }
		  }
	  });
}

bool ShadowRenderer::UpdateShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light, rendering::ShadowMapType smType)
{
	// TODO: Remove me
#if 0
	m_shadowCasters.clear();
	auto hShadowMap = light.GetShadowMap<pragma::CShadowComponent>(smType);
	if(hShadowMap.expired())
		return false;
	auto frameId = pragma::get_cengine()->GetRenderContext().GetLastFrameId();
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
		if(smType == pragma::rendering::ShadowMapType::Static)
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
	case pragma::rendering::ShadowMapType::Static:
		UpdateWorldShadowCasters(drawCmd,light);
		break;
	case pragma::rendering::ShadowMapType::Dynamic:
		UpdateEntityShadowCasters(drawCmd,light);
		break;
	}
#endif
	return true;
}

ShadowRenderer::RenderResultFlags ShadowRenderer::RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light, uint32_t layerId, const Mat4 &depthMVP, ShaderShadow &shader, bool bTranslucent)
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
		material::Material *prevMat = nullptr;
		for(auto &info : m_shadowCasters)
		{
			if(info.entity != nullptr)
			{
				bProcessMeshes = ((info.renderFlags &layerFlag) != 0) ? true : false;
				if(bProcessMeshes == true)
					shader.BindEntity(*const_cast<pragma::ecs::CBaseEntity*>(info.entity),depthMVP);
			}
			if(info.mesh != nullptr && bProcessMeshes == true)
			{
				if(info.material != nullptr && info.material->IsTranslucent())
				{
					if(bTranslucent == true)
					{
						auto &shaderTranslucent = static_cast<pragma::ShaderShadowTransparent&>(shader);
						if(info.material == prevMat || shaderTranslucent.BindMaterial(static_cast<material::CMaterial&>(*info.material)) == true)
						{
							shaderTranslucent.Draw(*const_cast<pragma::geometry::CModelSubMesh*>(info.mesh));
							prevMat = info.material;
						}
					}
					else
						hasTranslucents = true;
				}
				else if(bTranslucent == false)
					shader.Draw(*const_cast<pragma::geometry::CModelSubMesh*>(info.mesh));
			}
		}
	}
	shader.EndDraw();
	return hasTranslucents ? ShadowRenderer::RenderResultFlags::TranslucentPending : ShadowRenderer::RenderResultFlags::None;
#endif
	return RenderResultFlags::None;
}

static auto cvParticleQuality = console::get_client_con_var("cl_render_particle_quality");
void ShadowRenderer::RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light, rendering::ShadowMapType smType, LightType type, bool drawParticleShadows)
{
	auto hShadowMap = light.GetShadowMap<CShadowComponent>(smType);
	if(hShadowMap.expired() || light.GetEffectiveShadowType() == BaseEnvLightComponent::ShadowType::None || UpdateShadowCasters(drawCmd, light, smType) == false)
		return;
	auto &shader = (type != LightType::Spot) ? static_cast<ShaderShadow &>(*m_shader.get()) : static_cast<ShaderShadow &>(*m_shaderSpot.get());
	ShaderShadowTransparent *shaderTransparent = nullptr;
	if(type != LightType::Spot)
		shaderTransparent = static_cast<ShaderShadowTransparent *>(m_shaderTransparent.expired() == false ? m_shaderTransparent.get() : nullptr);
	else
		shaderTransparent = static_cast<ShaderShadowTransparent *>(m_shaderSpotTransparent.expired() == false ? m_shaderSpotTransparent.get() : nullptr);
#if DEBUG_SHADOWS == 1
	s_shadowMeshCount = 0;
	s_shadowIndexCount = 0;
	s_shadowTriangleCount = 0;
	s_shadowVertexCount = 0;
#endif

	auto *smRt = hShadowMap->GetDepthRenderTarget();
	auto &tex = smRt->GetTexture();
	auto *scene = get_cgame()->GetScene<CSceneComponent>();
	auto *renderer = scene ? scene->GetRenderer<CRendererComponent>() : nullptr;
	auto raster = renderer ? renderer->GetEntity().GetComponent<CRasterizationRendererComponent>() : pragma::ComponentHandle<CRasterizationRendererComponent> {};
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
		//if(pragma::math::is_flag_set(renderResultFlags,RenderResultFlags::TranslucentPending) && shaderTransparent != nullptr)
		//	RenderShadows(drawCmd,light,layerId,depthMVP,shader,true); // Draw translucent shadows
		if(drawParticleShadows == true && renderer) {
			auto *scene = get_cgame()->GetRenderScene<CSceneComponent>();
			// TODO: Only culled particles
			ecs::EntityIterator entIt {*get_cgame()};
			entIt.AttachFilter<TEntityIteratorFilterComponent<ecs::CParticleSystemComponent>>();
			for(auto *ent : entIt) {
				auto p = ent->GetComponent<ecs::CParticleSystemComponent>();
				if(p.valid() && p->GetCastShadows() == true)
					p->RecordRenderShadow(*drawCmd, *scene, *raster, &light, layerId);
			}
		}
		drawCmd->RecordEndRenderPass();

		prosper::util::ImageSubresourceRange range {layerId};
		drawCmd->RecordPostRenderPassImageBarrier(img, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, range);
	}
}

void ShadowRenderer::RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light)
{
	auto type = LightType::Undefined;
	auto *pLight = light.GetLight(type);
	auto bDrawParticleShadows = (cvParticleQuality->GetInt() >= 3) ? true : false;
	if(type == LightType::Directional) {
		RenderCSMShadows(drawCmd, static_cast<CLightDirectionalComponent &>(*pLight), bDrawParticleShadows);
		return;
	}
	if(m_shader.expired() || m_shaderSpot.expired())
		return;
	RenderShadows(drawCmd, light, rendering::ShadowMapType::Static, type, bDrawParticleShadows);
	RenderShadows(drawCmd, light, rendering::ShadowMapType::Dynamic, type, bDrawParticleShadows);
}
