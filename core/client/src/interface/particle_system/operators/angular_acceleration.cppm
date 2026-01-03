// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_angular_acceleration;

export import :particle_system.modifier;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorAngularAcceleration : public CParticleOperator {
	  public:
		CParticleOperatorAngularAcceleration() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	  private:
		Vector3 m_vAcceleration = {};
	};
}

void pragma::pts::CParticleOperatorAngularAcceleration::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "acceleration")
			m_vAcceleration = uvec::create(pair.second);
	}
}
void pragma::pts::CParticleOperatorAngularAcceleration::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	particle.SetAngularVelocity(particle.GetAngularVelocity() + m_vAcceleration * static_cast<float>(tDelta));
}
