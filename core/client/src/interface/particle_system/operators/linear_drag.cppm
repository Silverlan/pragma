// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_linear_drag;

export import :particle_system.modifier;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorLinearDrag : public CParticleOperator {
	  public:
		CParticleOperatorLinearDrag() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
		virtual void Simulate(double tDelta) override;
	  private:
		float m_fAmount = 1.f;
		float m_fTickDrag = 1.f;
	};
}

void pragma::pts::CParticleOperatorLinearDrag::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == "drag")
			m_fAmount = util::to_float(it->second);
	}
}
void pragma::pts::CParticleOperatorLinearDrag::Simulate(double tDelta)
{
	CParticleOperator::Simulate(tDelta);
	m_fTickDrag = math::max(0.f, 1.f - m_fAmount * static_cast<float>(tDelta));
}
void pragma::pts::CParticleOperatorLinearDrag::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	particle.SetVelocity(particle.GetVelocity() * m_fTickDrag);
}
