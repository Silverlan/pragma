// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "shader/prosper_shader.hpp"


export module pragma.client:particle_system.renderer_sprite_trail;

import :particle_system.modifier;

export class DLLCLIENT CParticleRendererSpriteTrail : public CParticleRenderer {
  public:
	CParticleRendererSpriteTrail() = default;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ecs::ParticleRenderFlags renderFlags) override;
	virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId = 0) override;
	virtual pragma::ShaderParticleBase *GetShader() const override;

	float GetMinLength() const { return m_minLength; }
	float GetMaxLength() const { return m_maxLength; }
	float GetLengthFadeInTime() const { return m_lengthFadeInTime; }
	float GetAnimationRate() const { return m_animationRate; }
  private:
	::util::WeakHandle<prosper::Shader> m_shader = {};
	float m_minLength = 0.f;
	float m_maxLength = 2'000.f;
	float m_lengthFadeInTime = 0.f;
	float m_animationRate = 0.1f;
};
