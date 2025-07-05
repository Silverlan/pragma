// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_INITIALIZER_SHOOT_HPP__
#define __C_PARTICLE_INITIALIZER_SHOOT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_random_variable.hpp"

class DLLCLIENT CParticleInitializerShootCone : public CParticleInitializer {
  public:
	CParticleInitializerShootCone() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
  private:
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fMinAngle;
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fMaxAngle;
	Vector3 m_vDirection = {1.f, 0.f, 0.f};
};

/////////////////////////

class DLLCLIENT CParticleInitializerShootOutward : public CParticleInitializer {
  public:
	CParticleInitializerShootOutward() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
  private:
	Vector3 m_vBias = {};
};

#endif
