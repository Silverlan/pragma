#ifndef __C_PARTICLE_INITIALIZER_COLOR_HPP__
#define __C_PARTICLE_INITIALIZER_COLOR_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_random_color.hpp"

class DLLCLIENT CParticleInitializerColor
	: public CParticleInitializer,
	public CParticleModifierComponentRandomColor
{
public:
	CParticleInitializerColor()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
};

#endif
