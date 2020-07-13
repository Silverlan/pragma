/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLE_MOD_MODEL_HPP__
#define __C_PARTICLE_MOD_MODEL_HPP__

#include "pragma/particlesystem/renderers/c_particle_renderer_rotational_buffer.hpp"

namespace pragma
{
	class CAnimatedComponent;
	class CLightComponent;
};
class DLLCLIENT CParticleRendererModel
	: public CParticleRenderer
{
public:
	CParticleRendererModel()=default;
	virtual ~CParticleRendererModel() override;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags) override;
	virtual void RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId=0) override;
	virtual void PostSimulate(double tDelta) override;
	virtual void OnParticleCreated(CParticle &particle) override;
	virtual pragma::ShaderParticleBase *GetShader() const override;

	bool IsAnimated() const;
protected:
	static std::size_t s_rendererCount;

	struct ParticleModelComponent
	{
		util::WeakHandle<pragma::CAnimatedComponent> animatedComponent;
		std::shared_ptr<prosper::IDescriptorSetGroup> instanceDescSetGroupAnimated;
	};
	ParticleModelComponent &GetParticleComponent(uint32_t particleIdx);
	std::vector<ParticleModelComponent> m_particleComponents;

	std::string m_animation;
	util::WeakHandle<prosper::Shader> m_shader = {};
	uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
	CParticleRendererRotationalBuffer m_rotationalBuffer;
	bool Update();
};

#endif
