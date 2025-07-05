// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_MOD_INITIAL_ANGULAR_VELOCITY_HPP__
#define __C_PARTICLE_MOD_INITIAL_ANGULAR_VELOCITY_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleInitializerInitialAngularVelocity : public CParticleInitializer {
  private:
	Vector3 m_direction = {};
	float m_speed = 0.f;
	Vector3 m_spreadMin = {};
	Vector3 m_spreadMax = {};

	struct RandomVelocity {
		Vector3 minVelocity = {};
		Vector3 maxVelocity = {};
	};
	std::unique_ptr<RandomVelocity> m_randomVelocity = nullptr;
  public:
	CParticleInitializerInitialAngularVelocity() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
};

#endif
