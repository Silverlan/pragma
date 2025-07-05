// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_RENDERER_SPRITE_TRAIL_HPP__
#define __C_PARTICLE_RENDERER_SPRITE_TRAIL_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/debug/c_debugoverlay.h"

namespace pragma {
	class CLightComponent;
};
class DLLCLIENT CParticleRendererSpriteTrail : public CParticleRenderer {
  public:
	CParticleRendererSpriteTrail() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags) override;
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

#endif
