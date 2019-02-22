#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_speed.hpp"

REGISTER_PARTICLE_INITIALIZER(speed,CParticleInitializerSpeed);

CParticleInitializerSpeed::CParticleInitializerSpeed(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleInitializer(pSystem,values),m_fSpeed("speed",values)
{
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "speed_min")
			m_fSpeed.SetMin(util::to_float(pair.second));
		else if(key == "speed_max")
			m_fSpeed.SetMax(util::to_float(pair.second));
	}
}
void CParticleInitializerSpeed::Initialize(CParticle &particle)
{
	auto vel = particle.GetVelocity();
	auto l = uvec::length(vel);
	if(l <= 0.0001f)
		return;
	vel /= l;
	particle.SetVelocity(vel *m_fSpeed.GetValue(particle));
}
