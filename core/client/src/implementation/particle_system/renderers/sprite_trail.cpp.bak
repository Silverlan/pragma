// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

module pragma.client.particle_system;

import :renderer_sprite_trail;
import :renderer_rotational_buffer;

import pragma.client.client_state;
import pragma.client.entities.components;
import pragma.client.game;

REGISTER_PARTICLE_RENDERER(source_render_sprite_trail, CParticleRendererSpriteTrail);

extern CEngine *c_engine;
extern CGame *c_game;

void CParticleRendererSpriteTrail::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleRenderer::Initialize(pSystem, values);
	auto bAlignVelocity = false;
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "min_length")
			m_minLength = util::to_float(pair.second);
		else if(key == "max_length")
			m_maxLength = util::to_float(pair.second);
		else if(key == "length_fade_in_time")
			m_lengthFadeInTime = util::to_float(pair.second);
		else if(key == "animation_rate")
			m_animationRate = util::to_float(pair.second);
	}
	m_shader = c_engine->GetShader("pfm_particle_sprite_trail");
}

pragma::ShaderParticleBase *CParticleRendererSpriteTrail::GetShader() const { return static_cast<pragma::ShaderParticle2DBase *>(m_shader.get()); }

void CParticleRendererSpriteTrail::RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags)
{
	auto *shader = static_cast<pragma::ShaderParticle2DBase *>(m_shader.get());
	prosper::ShaderBindState bindState {drawCmd};
	if(!shader)
		return;
	auto pipelineIdx = shader->RecordBeginDraw(bindState, GetParticleSystem(), renderFlags);
	if(!pipelineIdx.has_value()) // prosper TODO: Use unlit pipeline if low shader quality?
		return;
	auto layout = c_engine->GetRenderContext().GetShaderPipelineLayout(*shader, *pipelineIdx);
	assert(layout != nullptr);
	auto *dsScene = scene.GetCameraDescriptorSetGraphics();
	auto *dsRenderer = renderer.GetRendererDescriptorSet();
	auto &dsRenderSettings = c_game->GetGlobalRenderSettingsDescriptorSet();
	auto *dsShadows = pragma::CShadowComponent::GetDescriptorSet();
	shader->RecordBindScene(bindState.commandBuffer, *layout, scene, renderer, *dsScene, *dsRenderer, dsRenderSettings, *dsShadows);
	shader->RecordDraw(bindState, scene, renderer, *m_particleSystem, m_particleSystem->GetOrientationType(), renderFlags);
	shader->RecordEndDraw(bindState);
}

void CParticleRendererSpriteTrail::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId)
{
	/*static auto hShader = c_engine->GetShader("particleshadow");
	if(!hShader.IsValid())
		return;
	auto &shader = static_cast<Shader::ParticleShadow&>(*hShader.get());
	if(shader.BeginDraw() == false)
		return;
	//auto &cam = *c_game->GetRenderCamera<pragma::CCameraComponent>();
	shader.Draw(m_particleSystem,light,layerId);
	shader.EndDraw();*/ // prosper TODO
}
