// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:particle_system.renderer_sprite;

export import :entities.components.particle_system;
export import :particle_system.modifier;
import :particle_system.renderer_rotational_buffer;

export namespace pragma {
	namespace ecs {
		class CParticleSystemComponent;
	}
};
export namespace pragma::pts {
	class DLLCLIENT CParticleRendererSprite : public CParticleRenderer {
	public:
		CParticleRendererSprite() = default;
		virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::pts::ParticleRenderFlags renderFlags) override;
		virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId = 0) override;
		virtual void PostSimulate(double tDelta) override;
		virtual pragma::ShaderParticleBase *GetShader() const override;
	private:
		pragma::util::WeakHandle<prosper::Shader> m_shader = {};
		std::unique_ptr<pragma::pts::CParticleRendererRotationalBuffer> m_rotationalBuffer = nullptr;
		bool m_bPlanarRotation = true;
	};
}
