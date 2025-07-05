// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_MOD_INITIAL_VELOCITY_H__
#define __C_PARTICLE_MOD_INITIAL_VELOCITY_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <optional>

class DLLCLIENT CParticleInitializerInitialVelocity : public CParticleInitializer {
  public:
	CParticleInitializerInitialVelocity() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
	float GetSpeed() const;
  private:
	Vector3 m_direction = {};
	float m_speed = 0.f;
	Vector3 m_spreadMin = {};
	Vector3 m_spreadMax = {};
	Vector3 m_velocityMin = {};
	Vector3 m_velocityMax = {};
};

#endif
