// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_random_emission_rate;

export import :entities.components.particle_system;
export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorRandomEmissionRate : public CParticleOperator {
	  public:
		CParticleOperatorRandomEmissionRate() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(double tDelta) override;
		virtual void OnParticleSystemStarted() override;
	  private:
		float GetInterval() const;
		void Reset();
		float m_fMinimum = 0.07f;
		float m_fMaximum = 0.2f;
		float m_fRemaining = 0.f;
	};
}

void pragma::pts::CParticleOperatorRandomEmissionRate::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "minimum")
			m_fMinimum = util::to_float(pair.second);
		else if(key == "maximum")
			m_fMaximum = util::to_float(pair.second);
	}
}
void pragma::pts::CParticleOperatorRandomEmissionRate::OnParticleSystemStarted()
{
	CParticleOperator::OnParticleSystemStarted();
	Reset();
	Simulate(0.f);
}
void pragma::pts::CParticleOperatorRandomEmissionRate::Reset() { m_fRemaining = GetInterval(); }
float pragma::pts::CParticleOperatorRandomEmissionRate::GetInterval() const { return math::max(0.f, math::random(m_fMinimum, m_fMaximum)); }
void pragma::pts::CParticleOperatorRandomEmissionRate::Simulate(double tDelta)
{
	CParticleOperator::Simulate(tDelta);

	auto &ps = GetParticleSystem();
	if((m_fRemaining -= tDelta) > 0.f) {
		ps.SetNextParticleEmissionCount(0u);
		return;
	}
	auto maximum = ps.GetMaxParticleCount();
	auto count = math::min(1u, maximum);
	while(count < maximum && (m_fRemaining += GetInterval()) < 0.f)
		++count;
	ps.SetNextParticleEmissionCount(count);
}
