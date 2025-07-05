// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_OPERATOR_JITTER_HPP__
#define __C_PARTICLE_OPERATOR_JITTER_HPP__

#include "pragma/particlesystem/operators/c_particle_operator_wander.hpp"

class DLLCLIENT CParticleOperatorJitter : public CParticleOperatorWander {
  public:
	CParticleOperatorJitter() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
};

#endif
