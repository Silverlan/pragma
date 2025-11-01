// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:particle_system.operator_quadratic_drag;

export import :particle_system.modifier;

export namespace pragma::ecs {class CParticleSystemComponent;}
export class DLLCLIENT CParticleOperatorQuadraticDrag : public CParticleOperator {
  public:
	CParticleOperatorQuadraticDrag() = default;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	virtual void Simulate(double tDelta) override;
  private:
	float m_fAmount = 1.f;
	float m_fTickDrag = 1.f;
};

void CParticleOperatorQuadraticDrag::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "drag")
			m_fAmount = util::to_float(it->second);
	}
}
void CParticleOperatorQuadraticDrag::Simulate(double tDelta)
{
	CParticleOperator::Simulate(tDelta);
	m_fTickDrag = m_fAmount * static_cast<float>(tDelta);
}
void CParticleOperatorQuadraticDrag::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	auto &velocity = particle.GetVelocity();
	particle.SetVelocity(velocity * umath::max(0.f, 1.f - m_fTickDrag * uvec::length(velocity)));
}
