// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.renderer_animated_sprites;

export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleRendererAnimatedSprites : public CParticleRenderer {
	  public:
		CParticleRendererAnimatedSprites() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags) override;
		virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId = 0) override;
		virtual ShaderParticleBase *GetShader() const override;
	  private:
		util::WeakHandle<prosper::Shader> m_shader = {};
	};
}
