// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.renderer_sprite_trail;

export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleRendererSpriteTrail : public CParticleRenderer {
	  public:
		CParticleRendererSpriteTrail() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags) override;
		virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId = 0) override;
		virtual ShaderParticleBase *GetShader() const override;

		float GetMinLength() const { return m_minLength; }
		float GetMaxLength() const { return m_maxLength; }
		float GetLengthFadeInTime() const { return m_lengthFadeInTime; }
		float GetAnimationRate() const { return m_animationRate; }
	  private:
		util::WeakHandle<prosper::Shader> m_shader = {};
		float m_minLength = 0.f;
		float m_maxLength = 2'000.f;
		float m_lengthFadeInTime = 0.f;
		float m_animationRate = 0.1f;
	};
}
