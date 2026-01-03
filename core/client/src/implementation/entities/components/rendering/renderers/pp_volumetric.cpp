// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components.pp_volumetric;
import :model;
import :engine;
import :entities.components.color;
import :entities.components.lights.spot_vol;
import :entities.components.radius;
import :entities.components.render;
import :game;
import :rendering.shaders;

using namespace pragma;

CRendererPpVolumetricComponent::CRendererPpVolumetricComponent(ecs::BaseEntity &ent) : CRendererPpBaseComponent(ent)
{
	static auto g_shadersRegistered = false;
	if(!g_shadersRegistered) {
		g_shadersRegistered = true;
		get_cengine()->GetShaderManager().RegisterShader("pp_light_cone", [](prosper::IPrContext &context, const std::string &identifier) { return new ShaderPPLightCone(context, identifier); });
	}
}
static util::WeakHandle<prosper::Shader> g_shader {};
void CRendererPpVolumetricComponent::OnEntitySpawn()
{
	CRendererPpBaseComponent::OnEntitySpawn();
	if(g_shader.expired())
		g_shader = get_cengine()->GetShader("pp_light_cone");
	ReloadRenderTarget();
}

void CRendererPpVolumetricComponent::ReloadRenderTarget()
{
	if(m_renderer.expired())
		return;
	get_cengine()->GetRenderContext().WaitIdle();
	auto &hdrInfo = m_renderer->GetHDRInfo();
	if(!hdrInfo.sceneRenderTarget)
		return;
	m_renderTarget = nullptr;
	auto rendererC = GetEntity().GetComponent<CRendererComponent>();
	auto cRenderer = GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rendererC.expired() || cRenderer.expired() || g_shader.expired())
		return;
	auto *shaderLightCone = static_cast<ShaderPPLightCone *>(g_shader.get());
	if(shaderLightCone && shaderLightCone->IsValid())
		m_renderTarget = get_cengine()->GetRenderContext().CreateRenderTarget({hdrInfo.sceneRenderTarget->GetTexture(0)->shared_from_this(), hdrInfo.bloomTexture, hdrInfo.prepass.textureDepth}, shaderLightCone->GetRenderPass());
}
void CRendererPpVolumetricComponent::Initialize()
{
	CRendererPpBaseComponent::Initialize();
	BindEventUnhandled(cRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(!GetEntity().IsSpawned())
			return;
		ReloadRenderTarget();
	});
	if(GetEntity().IsSpawned())
		ReloadRenderTarget();
}
void CRendererPpVolumetricComponent::DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired() || m_renderer.expired() || g_shader.expired() || !m_renderTarget)
		return;
	auto &shaderLightCone = *static_cast<ShaderPPLightCone *>(g_shader.get());
	auto &scene = *drawSceneInfo.scene;
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &cam = scene.GetActiveCamera();
	auto *dsCam = scene.GetCameraDescriptorSetGraphics();
	if(cam.expired() || !dsCam)
		return;

	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	ShaderPPLightCone::PushConstants pushConstants {};
	pushConstants.nearZ = cam->GetNearZ();
	pushConstants.farZ = cam->GetFarZ();
	pushConstants.SetResolution(m_renderTarget->GetTexture().GetImage().GetWidth(), m_renderTarget->GetTexture().GetImage().GetHeight());
	auto &frustumPlanes = cam->GetFrustumPlanes();

	ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightSpotVolComponent>>();
	std::vector<ecs::BaseEntity *> ents;
	ents.reserve(entIt.GetCount());
	for(auto *ent : entIt) {
		auto renderC = ent->GetComponent<CRenderComponent>();
		if(renderC.expired())
			continue;
		// Make sure render buffers are up to date
		renderC->UpdateRenderBuffers(drawCmd);
		ents.push_back(ent);
	}

	constexpr float lightIntensityFactor = 0.01f;
	if(drawCmd->RecordBeginRenderPass(*m_renderTarget) == true) {
		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderLightCone.RecordBeginDraw(bindState) == true) {
			for(auto *ent : ents) {
				auto volC = ent->GetComponent<CLightSpotVolComponent>();
				auto mdlC = ent->GetComponent<CModelComponent>();
				auto renderC = ent->GetComponent<CRenderComponent>();
				auto radiusC = ent->GetComponent<CRadiusComponent>();
				auto colorC = ent->GetComponent<CColorComponent>();
				auto lightC = ent->GetComponent<CLightComponent>();
				if(mdlC.expired() || renderC.expired() || radiusC.expired())
					continue;
				auto *dsInstance = renderC->GetRenderDescriptorSet();
				if(!dsInstance)
					continue;
				pushConstants.color = colorC.valid() ? colorC->GetColor() : colors::White.ToVector4();
				pushConstants.color = Vector4 {image::linear_to_srgb(reinterpret_cast<Vector3 &>(pushConstants.color)), pushConstants.color.w};
				pushConstants.color.w = lightC.valid() ? (lightC->GetLightIntensityCandela() * lightIntensityFactor) : 1.f;
				pushConstants.color.w *= volC->GetIntensityFactor();
				pushConstants.coneOrigin = ent->GetPosition();
				pushConstants.coneLength = radiusC->GetRadius();
				shaderLightCone.RecordPushConstants(bindState, pushConstants);

				auto &renderMeshes = mdlC->GetRenderMeshes();
				for(auto &mesh : renderMeshes) {
					auto &cmesh = static_cast<geometry::CModelSubMesh &>(*mesh);

					// TODO: We only have to bind the instance and camera descriptor sets once per entity and not for every mesh
					shaderLightCone.RecordDraw(bindState, cmesh, *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(), *hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(), *dsInstance, *dsCam);
				}
			}
			shaderLightCone.RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
}
void CRendererPpVolumetricComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
