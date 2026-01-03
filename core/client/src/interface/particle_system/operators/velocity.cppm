// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_velocity;

export import :particle_system.modifier;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorVelocity : public CParticleOperator {
	  private:
		Vector3 m_velocity = {};
	  public:
		CParticleOperatorVelocity() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
		float GetSpeed() const;
	};
}

void pragma::pts::CParticleOperatorVelocity::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		string::to_lower(key);
		if(key == "velocity")
			m_velocity = uvec::create(it->second);
	}
}
void pragma::pts::CParticleOperatorVelocity::Simulate(CParticle &particle, double tDelta, float strength)
{
	Vector3 vel = particle.GetVelocity();
	vel += m_velocity * (float)tDelta;
	particle.SetVelocity(vel);
}
float pragma::pts::CParticleOperatorVelocity::GetSpeed() const { return uvec::length(m_velocity); }
