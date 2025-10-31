// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:particle_system.operator_jitter;

import :particle_system.operator_wander;

export namespace pragma::ecs {class CParticleSystemComponent;}
export class DLLCLIENT CParticleOperatorJitter : public CParticleOperatorWander {
  public:
	CParticleOperatorJitter() = default;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
};

void CParticleOperatorJitter::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) { CParticleOperatorWander::Initialize(pSystem, values); }
void CParticleOperatorJitter::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperatorWorldBase::Simulate(particle, tDelta, strength);

	// jitter is just like wander, except it directly influences the position
	auto pid = m_hashCodes.at(particle.GetIndex());
	auto time = m_dtTime + (pid & 255) / 256.f;
	particle.SetPosition(particle.GetPosition() + Vector3(util::noise::get_noise(time, pid) * m_dtStrength, util::noise::get_noise(time, pid + 1) * m_dtStrength, util::noise::get_noise(time, pid + 2) * m_dtStrength));
}
