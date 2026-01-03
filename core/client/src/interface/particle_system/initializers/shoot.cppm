// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.initializer_shoot;

export import :particle_system.modifier;
import :particle_system.modifier_random_variable;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleInitializerShootCone : public CParticleInitializer {
	  public:
		CParticleInitializerShootCone() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  private:
		CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fMinAngle;
		CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fMaxAngle;
		Vector3 m_vDirection = {1.f, 0.f, 0.f};
	};

	/////////////////////////

	class DLLCLIENT CParticleInitializerShootOutward : public CParticleInitializer {
	  public:
		CParticleInitializerShootOutward() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  private:
		Vector3 m_vBias = {};
	};
};
