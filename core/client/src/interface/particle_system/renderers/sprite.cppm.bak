// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <sharedutils/util_weak_handle.hpp>

export module pragma.client.particle_system:renderer_sprite;

import :renderer_rotational_buffer;

export class DLLCLIENT CParticleRendererSprite : public CParticleRenderer {
  public:
	CParticleRendererSprite() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags) override;
	virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId = 0) override;
	virtual void PostSimulate(double tDelta) override;
	virtual pragma::ShaderParticleBase *GetShader() const override;
  private:
	::util::WeakHandle<prosper::Shader> m_shader = {};
	std::unique_ptr<CParticleRendererRotationalBuffer> m_rotationalBuffer = nullptr;
	bool m_bPlanarRotation = true;
};
