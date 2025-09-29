// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "prosper_descriptor_set_group.hpp"

export module pragma.client:particle_system.renderer_model;

import :entities.components.animated;
import :particle_system.modifier;
import :particle_system.renderer_rotational_buffer;

export namespace pragma {
	enum class ParticleRenderFlags : uint32_t;
	namespace ecs {class CParticleSystemComponent;}
};
export class DLLCLIENT CParticleRendererModel : public CParticleRenderer {
  public:
	CParticleRendererModel() = default;
	virtual ~CParticleRendererModel() override;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ecs::ParticleRenderFlags renderFlags) override;
	virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId = 0) override;
	virtual void PostSimulate(double tDelta) override;
	virtual void OnParticleCreated(CParticle &particle) override;
	virtual pragma::ShaderParticleBase *GetShader() const override;

	bool IsAnimated() const;
  protected:
	static std::size_t s_rendererCount;

	struct ParticleModelComponent {
		pragma::ComponentHandle<pragma::CAnimatedComponent> animatedComponent;
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
