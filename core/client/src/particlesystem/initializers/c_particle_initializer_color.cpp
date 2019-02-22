#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_color.hpp"

REGISTER_PARTICLE_INITIALIZER(color_random,CParticleInitializerColor);

CParticleInitializerColor::CParticleInitializerColor(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleInitializer(pSystem,values),CParticleModifierComponentRandomColor("",values)
{}
void CParticleInitializerColor::Initialize(CParticle &particle) {particle.SetColor(GetValue(particle));}
