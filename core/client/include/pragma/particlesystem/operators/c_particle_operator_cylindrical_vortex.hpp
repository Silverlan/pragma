// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_OPERATOR_CYLINDRICAL_VORTEX_HPP__
#define __C_PARTICLE_OPERATOR_CYLINDRICAL_VORTEX_HPP__

#include "pragma/particlesystem/operators/c_particle_operator_world_base.hpp"

class DLLCLIENT CParticleOperatorCylindricalVortex : public CParticleOperatorWorldBase {
  public:
	CParticleOperatorCylindricalVortex() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	virtual void Simulate(double tDelta) override;
  private:
	Vector3 m_vAxis = {0.f, 1.f, 0.f};
	float m_fStrength = 2.f;
	float m_fDivergence = 0.f;

	float m_dtStrength = 0.f;
	Vector3 m_dtOrigin = {};
	Vector3 m_dtAxis = {};
	Quat m_dtRotation = uquat::identity();
};

#endif
