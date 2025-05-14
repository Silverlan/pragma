/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/renderers/c_particle_renderer_animated_sprites.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_shadow.h"
#include "pragma/particlesystem/renderers/c_particle_renderer_rotational_buffer.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

REGISTER_PARTICLE_RENDERER(source_render_animated_sprites, CParticleRendererAnimatedSprites);

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void CParticleRendererAnimatedSprites::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleRenderer::Initialize(pSystem, values);
	auto bAlignVelocity = false;
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		// self.m_orientationType = tonumber(self:GetKeyValue("orientation_type") or "") or 0
		// self.m_orientationControlPoint = tonumber(self:GetKeyValue("control_point_id") or "") or -1
		// self.m_secondAnimationRate = tonumber(self:GetKeyValue("second_sequence_animation_rate") or "") or 0.0
	}
	m_shader = c_engine->GetShader("pfm_particle_animated_sprites");
}

pragma::ShaderParticleBase *CParticleRendererAnimatedSprites::GetShader() const { return static_cast<pragma::ShaderParticle2DBase *>(m_shader.get()); }

void CParticleRendererAnimatedSprites::RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags)
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

void CParticleRendererAnimatedSprites::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId)
{
	/*static auto hShader = c_engine->GetShader("particleshadow");
	if(!hShader.IsValid())
		return;
	auto &shader = static_cast<Shader::ParticleShadow&>(*hShader.get());
	if(shader.BeginDraw() == false)
		return;
	//auto &cam = *c_game->GetRenderCamera();
	shader.Draw(m_particleSystem,light,layerId);
	shader.EndDraw();*/ // prosper TODO
}
