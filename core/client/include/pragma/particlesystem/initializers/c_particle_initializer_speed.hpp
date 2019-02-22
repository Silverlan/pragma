#ifndef __C_PARTICLE_INITIALIZER_SPEED_HPP__
#define __C_PARTICLE_INITIALIZER_SPEED_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_random_variable.hpp"

class DLLCLIENT CParticleInitializerSpeed
	: public CParticleInitializer
{
public:
	CParticleInitializerSpeed(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Initialize(CParticle &particle) override;
private:
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>,float> m_fSpeed;
};

#endif
