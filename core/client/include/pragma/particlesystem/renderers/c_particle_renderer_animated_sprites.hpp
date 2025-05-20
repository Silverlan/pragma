/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
