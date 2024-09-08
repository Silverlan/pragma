/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_renderer_pp_volumetric_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/components/renderers/rasterization/hdr_data.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_light_cone.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <prosper_command_buffer.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <util_image.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

CRendererPpVolumetricComponent::CRendererPpVolumetricComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent)
{
	static auto g_shadersRegistered = false;
	if(!g_shadersRegistered) {
		g_shadersRegistered = true;
		c_engine->GetShaderManager().RegisterShader("pp_light_cone", [](prosper::IPrContext &context, const std::string &identifier) { return new pragma::ShaderPPLightCone(context, identifier); });
	}
}
static util::WeakHandle<prosper::Shader> g_shader {};
void CRendererPpVolumetricComponent::OnEntitySpawn()
{
	CRendererPpBaseComponent::OnEntitySpawn();
	if(g_shader.expired())
		g_shader = c_engine->GetShader("pp_light_cone");
	ReloadRenderTarget();
}

void CRendererPpVolumetricComponent::ReloadRenderTarget()
{
	if(m_renderer.expired())
		return;
	c_engine->GetRenderContext().WaitIdle();
	auto &hdrInfo = m_renderer->GetHDRInfo();
	if(!hdrInfo.sceneRenderTarget)
		return;
	m_renderTarget = nullptr;
	auto rendererC = GetEntity().GetComponent<pragma::CRendererComponent>();
	auto cRenderer = GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rendererC.expired() || cRenderer.expired() || g_shader.expired())
		return;
	auto *shaderLightCone = static_cast<pragma::ShaderPPLightCone *>(g_shader.get());
	if(shaderLightCone && shaderLightCone->IsValid())
		m_renderTarget = c_engine->GetRenderContext().CreateRenderTarget({hdrInfo.sceneRenderTarget->GetTexture(0)->shared_from_this(), hdrInfo.bloomTexture, hdrInfo.prepass.textureDepth}, shaderLightCone->GetRenderPass());
}
void CRendererPpVolumetricComponent::Initialize()
{
	CRendererPpBaseComponent::Initialize();
	BindEventUnhandled(pragma::CRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(!GetEntity().IsSpawned())
			return;
		ReloadRenderTarget();
	});
	if(GetEntity().IsSpawned())
		ReloadRenderTarget();
}
void CRendererPpVolumetricComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired() || m_renderer.expired() || g_shader.expired() || !m_renderTarget)
		return;
	auto &shaderLightCone = *static_cast<pragma::ShaderPPLightCone *>(g_shader.get());
	auto &scene = *drawSceneInfo.scene;
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &cam = scene.GetActiveCamera();
	auto *dsCam = scene.GetCameraDescriptorSetGraphics();
	if(cam.expired() || !dsCam)
		return;

	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	pragma::ShaderPPLightCone::PushConstants pushConstants {};
	pushConstants.nearZ = cam->GetNearZ();
	pushConstants.farZ = cam->GetFarZ();
	pushConstants.SetResolution(m_renderTarget->GetTexture().GetImage().GetWidth(), m_renderTarget->GetTexture().GetImage().GetHeight());
	auto &frustumPlanes = cam->GetFrustumPlanes();

	EntityIterator entIt {*c_game, EntityIterator::FilterFlags::Default};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightSpotVolComponent>>();
	std::vector<BaseEntity *> ents;
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
				auto volC = ent->GetComponent<pragma::CLightSpotVolComponent>();
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
				pushConstants.color = colorC.valid() ? colorC->GetColor() : Color::White.ToVector4();
				pushConstants.color = Vector4 {uimg::linear_to_srgb(reinterpret_cast<Vector3 &>(pushConstants.color)), pushConstants.color.w};
				pushConstants.color.w = lightC.valid() ? (lightC->GetLightIntensityCandela() * lightIntensityFactor) : 1.f;
				pushConstants.color.w *= volC->GetIntensityFactor();
				pushConstants.coneOrigin = ent->GetPosition();
				pushConstants.coneLength = radiusC->GetRadius();
				shaderLightCone.RecordPushConstants(bindState, pushConstants);

				auto &renderMeshes = mdlC->GetRenderMeshes();
				for(auto &mesh : renderMeshes) {
					auto &cmesh = static_cast<CModelSubMesh &>(*mesh);

					// TODO: We only have to bind the instance and camera descriptor sets once per entity and not for every mesh
					shaderLightCone.RecordDraw(bindState, cmesh, *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(), *hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(), *dsInstance, *dsCam);
				}
			}
			shaderLightCone.RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
}
void CRendererPpVolumetricComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
