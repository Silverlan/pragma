// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_RENDERER_ANIMATED_SPRITES_HPP__
#define __C_PARTICLE_RENDERER_ANIMATED_SPRITES_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/debug/c_debugoverlay.h"

namespace pragma {
	class CLightComponent;
};
class DLLCLIENT CParticleRendererAnimatedSprites : public CParticleRenderer {
  public:
	CParticleRendererAnimatedSprites() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags) override;
	virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId = 0) override;
	virtual pragma::ShaderParticleBase *GetShader() const override;
  private:
	::util::WeakHandle<prosper::Shader> m_shader = {};
};

#endif
