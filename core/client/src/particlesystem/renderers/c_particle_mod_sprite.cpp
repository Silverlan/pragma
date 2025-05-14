/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/renderers/c_particle_mod_sprite.h"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_shadow.h"
#include "pragma/particlesystem/renderers/c_particle_renderer_rotational_buffer.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

REGISTER_PARTICLE_RENDERER(sprite, CParticleRendererSprite);

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void CParticleRendererSprite::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
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
	m_shader = c_engine->GetShader(m_bPlanarRotation ? "particle" : "particle_rotational");
	if(m_bPlanarRotation == true)
		return;
	m_rotationalBuffer = std::make_unique<CParticleRendererRotationalBuffer>();
	m_rotationalBuffer->Initialize(pSystem);
	m_rotationalBuffer->SetRotationAlignVelocity(bAlignVelocity);
}

void CParticleRendererSprite::PostSimulate(double tDelta)
{
	CParticleRenderer::PostSimulate(tDelta);
	if(m_shader.expired() || m_rotationalBuffer == nullptr)
		return;
	m_rotationalBuffer->Update();
}

pragma::ShaderParticleBase *CParticleRendererSprite::GetShader() const { return static_cast<pragma::ShaderParticle2DBase *>(m_shader.get()); }

void CParticleRendererSprite::RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags)
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
	shader->RecordDraw(bindState, scene, renderer, *m_particleSystem, (m_rotationalBuffer != nullptr && m_rotationalBuffer->ShouldRotationAlignVelocity()) ? pragma::CParticleSystemComponent::OrientationType::Velocity : m_particleSystem->GetOrientationType(), renderFlags);
	shader->RecordEndDraw(bindState);
}

void CParticleRendererSprite::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId)
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
