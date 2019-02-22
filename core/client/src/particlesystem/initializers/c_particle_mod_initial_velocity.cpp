#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/initializers/c_particle_mod_initial_velocity.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_INITIALIZER(initial_velocity,CParticleInitializerInitialVelocity);

CParticleInitializerInitialVelocity::CParticleInitializerInitialVelocity(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleInitializer(pSystem,values),
	m_speed(0)
{
	for(auto it=values.begin();it!=values.end();it++)
	{
		std::string key = it->first;
		std::transform(key.begin(),key.end(),key.begin(),::tolower);
		if(key == "velocity")
		{
			Vector3 velocity;
			velocity = uvec::create(it->second);
			m_speed = uvec::length(velocity);
			if(m_speed != 0.f)
				m_direction = uvec::get_normal(velocity);
			else
				m_direction = {};
		}
		else if(key == "spread_min")
			m_spreadMin = uvec::create(it->second);
		else if(key == "spread_max")
			m_spreadMax = uvec::create(it->second);
	}
}
void CParticleInitializerInitialVelocity::Initialize(CParticle &particle)
{
	if(m_speed == 0.f)
		return;
	Vector3 dir = glm::normalize(m_direction +uvec::get_random_spread(m_spreadMin,m_spreadMax));
	particle.SetVelocity(dir *m_speed);
}
float CParticleInitializerInitialVelocity::GetSpeed() const {return m_speed;}
