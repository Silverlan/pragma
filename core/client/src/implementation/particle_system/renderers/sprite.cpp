// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :particle_system.enums;
import :particle_system.renderer_sprite;
import :particle_system.renderer_rotational_buffer;

import :client_state;
import :entities.components;
import :game;

void pragma::pts::CParticleRendererSprite::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleRenderer::Initialize(pSystem, values);
	auto bAlignVelocity = false;
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "planar_rotation")
			m_bPlanarRotation = util::to_boolean(pair.second);
		else if(key == "align_velocity")
			bAlignVelocity = util::to_boolean(pair.second);
	}
	m_shader = pragma::get_cengine()->GetShader(m_bPlanarRotation ? "particle" : "particle_rotational");
	if(m_bPlanarRotation == true)
		return;
	m_rotationalBuffer = std::make_unique<pragma::pts::CParticleRendererRotationalBuffer>();
	m_rotationalBuffer->Initialize(pSystem);
	m_rotationalBuffer->SetRotationAlignVelocity(bAlignVelocity);
}

void pragma::pts::CParticleRendererSprite::PostSimulate(double tDelta)
{
	CParticleRenderer::PostSimulate(tDelta);
	if(m_shader.expired() || m_rotationalBuffer == nullptr)
		return;
	m_rotationalBuffer->Update();
}

pragma::ShaderParticleBase *pragma::pts::CParticleRendererSprite::GetShader() const { return static_cast<pragma::ShaderParticle2DBase *>(m_shader.get()); }

void pragma::pts::CParticleRendererSprite::RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::pts::ParticleRenderFlags renderFlags)
{
	auto *shader = static_cast<pragma::ShaderParticle2DBase *>(m_shader.get());
	prosper::ShaderBindState bindState {drawCmd};
	if(!shader)
		return;
	auto pipelineIdx = shader->RecordBeginDraw(bindState, GetParticleSystem(), renderFlags);
	if(!pipelineIdx.has_value()) // prosper TODO: Use unlit pipeline if low shader quality?
		return;
	auto layout = pragma::get_cengine()->GetRenderContext().GetShaderPipelineLayout(*shader, *pipelineIdx);
	assert(layout != nullptr);
	auto *dsScene = scene.GetCameraDescriptorSetGraphics();
	auto *dsRenderer = renderer.GetRendererDescriptorSet();
	auto &dsRenderSettings = pragma::get_cgame()->GetGlobalRenderSettingsDescriptorSet();
	auto *dsShadows = pragma::CShadowComponent::GetDescriptorSet();
	shader->RecordBindScene(bindState.commandBuffer, *layout, scene, renderer, *dsScene, *dsRenderer, dsRenderSettings, *dsShadows);
	auto orientationType = (m_rotationalBuffer != nullptr && m_rotationalBuffer->ShouldRotationAlignVelocity()) ? pragma::pts::ParticleOrientationType::Velocity : m_particleSystem->GetOrientationType();
	shader->RecordDraw(bindState, scene, renderer, *m_particleSystem, orientationType, renderFlags);
	shader->RecordEndDraw(bindState);
}

void pragma::pts::CParticleRendererSprite::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId)
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
