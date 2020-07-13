/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLE_MOD_SPRITE_H__
#define __C_PARTICLE_MOD_SPRITE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <sharedutils/util_weak_handle.hpp>

class CParticleRendererRotationalBuffer;
namespace pragma {class CLightComponent;};
class DLLCLIENT CParticleRendererSprite
	: public CParticleRenderer
{
public:
	CParticleRendererSprite()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags) override;
	virtual void RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId=0) override;
	virtual void PostSimulate(double tDelta) override;
	virtual pragma::ShaderParticleBase *GetShader() const override;
private:
	::util::WeakHandle<prosper::Shader> m_shader = {};
	std::unique_ptr<CParticleRendererRotationalBuffer> m_rotationalBuffer = nullptr;
	bool m_bPlanarRotation = true;
};

#endif