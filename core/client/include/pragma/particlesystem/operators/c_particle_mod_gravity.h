// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_MOD_GRAVITY_H__
#define __C_PARTICLE_MOD_GRAVITY_H__

#include "pragma/particlesystem/operators/c_particle_operator_world_base.hpp"

class DLLCLIENT CParticleOperatorGravity : public CParticleOperatorWorldBase {
  public:
	CParticleOperatorGravity() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	virtual void Simulate(double tDelta) override;
  protected:
	float m_gravityScale = 1.f;
	Vector3 m_gravityForce = {0.f, -1.f, 0.f};
	bool m_bUseCustomGravityForce = false;

	Vector3 m_dtGravity = {};
};

#endif
