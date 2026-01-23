// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.initializer_initial_angular_velocity;

export import :particle_system.modifier;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleInitializerInitialAngularVelocity : public CParticleInitializer {
	  private:
		Vector3 m_direction = {};
		float m_speed = 0.f;
		Vector3 m_spreadMin = {};
		Vector3 m_spreadMax = {};

		struct RandomVelocity {
			Vector3 minVelocity = {};
			Vector3 maxVelocity = {};
		};
		std::unique_ptr<RandomVelocity> m_randomVelocity = nullptr;
	  public:
		CParticleInitializerInitialAngularVelocity() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	};
}
