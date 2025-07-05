// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_OPERATOR_WIND_HPP__
#define __C_PARTICLE_OPERATOR_WIND_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorWind : public CParticleOperator {
  public:
	CParticleOperatorWind() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	virtual void Simulate(double tDelta) override;
  private:
	bool m_bRotateWithEmitter = false;
	float m_fStrength = 2.f;
	Vector3 m_vDirection = {1.f, 0.f, 0.f};
	Vector3 m_vDelta = {};
};

#endif
