// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>

export module pragma.client.particle_system:operator_angular_acceleration;

export class DLLCLIENT CParticleOperatorAngularAcceleration : public CParticleOperator {
  public:
	CParticleOperatorAngularAcceleration() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
  private:
	Vector3 m_vAcceleration = {};
};

REGISTER_PARTICLE_OPERATOR(angular_acceleration, CParticleOperatorAngularAcceleration);

void CParticleOperatorAngularAcceleration::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "acceleration")
			m_vAcceleration = uvec::create(pair.second);
	}
}
void CParticleOperatorAngularAcceleration::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	particle.SetAngularVelocity(particle.GetAngularVelocity() + m_vAcceleration * static_cast<float>(tDelta));
}
