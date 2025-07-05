// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_OPERATOR_RANDOM_EMISSION_RATE_HPP__
#define __C_PARTICLE_OPERATOR_RANDOM_EMISSION_RATE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorRandomEmissionRate : public CParticleOperator {
  public:
	CParticleOperatorRandomEmissionRate() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(double tDelta) override;
	virtual void OnParticleSystemStarted() override;
  private:
	float GetInterval() const;
	void Reset();
	float m_fMinimum = 0.07f;
	float m_fMaximum = 0.2f;
	float m_fRemaining = 0.f;
};

#endif
