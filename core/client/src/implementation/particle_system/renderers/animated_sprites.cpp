// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :particle_system.renderer_animated_sprites;
import :particle_system.renderer_rotational_buffer;

import :client_state;
import :entities.components;
import :game;

void pragma::pts::CParticleRendererAnimatedSprites::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleRenderer::Initialize(pSystem, values);
	auto bAlignVelocity = false;
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		// self.m_orientationType = tonumber(self:GetKeyValue("orientation_type") or "") or 0
		// self.m_orientationControlPoint = tonumber(self:GetKeyValue("control_point_id") or "") or -1
		// self.m_secondAnimationRate = tonumber(self:GetKeyValue("second_sequence_animation_rate") or "") or 0.0
	}
	m_shader = get_cengine()->GetShader("pfm_particle_animated_sprites");
}

pragma::ShaderParticleBase *pragma::pts::CParticleRendererAnimatedSprites::GetShader() const { return static_cast<ShaderParticle2DBase *>(m_shader.get()); }

void pragma::pts::CParticleRendererAnimatedSprites::RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags)
{
	auto *shader = static_cast<ShaderParticle2DBase *>(m_shader.get());
	prosper::ShaderBindState bindState {drawCmd};
	if(!shader)
		return;
	auto pipelineIdx = shader->RecordBeginDraw(bindState, GetParticleSystem(), renderFlags);
	if(!pipelineIdx.has_value()) // prosper TODO: Use unlit pipeline if low shader quality?
		return;
	auto layout = get_cengine()->GetRenderContext().GetShaderPipelineLayout(*shader, *pipelineIdx);
	assert(layout != nullptr);
	auto *dsScene = scene.GetCameraDescriptorSetGraphics();
	auto *dsRenderer = renderer.GetRendererDescriptorSet();
	auto &dsRenderSettings = get_cgame()->GetGlobalRenderSettingsDescriptorSet();
	auto *dsShadows = CShadowComponent::GetDescriptorSet();
	shader->RecordBindScene(bindState.commandBuffer, *layout, scene, renderer, *dsScene, *dsRenderer, dsRenderSettings, *dsShadows);
	shader->RecordDraw(bindState, scene, renderer, *m_particleSystem, m_particleSystem->GetOrientationType(), renderFlags);
	shader->RecordEndDraw(bindState);
}

void pragma::pts::CParticleRendererAnimatedSprites::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId)
{
	/*static auto hShader = pragma::get_cengine()->GetShader("particleshadow");
	if(!hShader.IsValid())
		return;
	auto &shader = static_cast<Shader::ParticleShadow&>(*hShader.get());
	if(shader.BeginDraw() == false)
		return;
	//auto &cam = *pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
	shader.Draw(m_particleSystem,light,layerId);
	shader.EndDraw();*/ // prosper TODO
}
