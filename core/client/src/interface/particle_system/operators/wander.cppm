// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_wander;

export import :entities.components.particle_system;
export import :particle_system.operator_world_base;

import :engine;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorWander : public CParticleOperatorWorldBase {
	  public:
		CParticleOperatorWander() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
		virtual void Simulate(double tDelta) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  protected:
		std::vector<int32_t> m_hashCodes;
		float m_fFrequency = 2.f;
		float m_fStrength = 0.05f;

		float m_dtTime = 0.f;
		float m_dtStrength = 0.f;
	};
}

void pragma::pts::CParticleOperatorWander::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorWorldBase::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == "strength")
			m_fStrength = util::to_float(it->second);
		else if(key == "frequency")
			m_fFrequency = util::to_float(it->second);
	}

	m_hashCodes.resize(static_cast<ecs::CParticleSystemComponent &>(pSystem).GetMaxParticleCount());
}
void pragma::pts::CParticleOperatorWander::OnParticleCreated(CParticle &particle) { m_hashCodes.at(particle.GetIndex()) = math::random(1, std::numeric_limits<int32_t>::max()); }
void pragma::pts::CParticleOperatorWander::Simulate(double tDelta)
{
	CParticleOperatorWorldBase::Simulate(tDelta);

	m_dtTime += tDelta * m_fFrequency;
	m_dtStrength = m_fStrength * tDelta * 60.f;
}
void pragma::pts::CParticleOperatorWander::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperatorWorldBase::Simulate(particle, tDelta, strength);

	// using the system hash gives each particle a consistent unique identity;
	// adding an offset to the time prevents synchronization of the zero points
	// (the noise function is always zero at integers)
	auto pid = m_hashCodes.at(particle.GetIndex());
	auto time = m_dtTime + (pid & 255) / 256.f;
	particle.SetVelocity(particle.GetVelocity() + Vector3(math::noise::get_noise(time, pid) * m_dtStrength, math::noise::get_noise(time, pid + 1) * m_dtStrength, math::noise::get_noise(time, pid + 2) * m_dtStrength));
}
