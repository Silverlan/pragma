#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/initializers/c_particle_mod_initial_angular_velocity.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_INITIALIZER(initial_angular_velocity,CParticleInitializerInitialAngularVelocity);

void CParticleInitializerInitialAngularVelocity::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleInitializer::Initialize(pSystem,values);
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "velocity")
		{
			auto velocity = uvec::create(pair.second);
			m_speed = uvec::length(velocity);
			if(m_speed != 0.f)
				m_direction = uvec::get_normal(velocity);
			else
				m_direction = {};
		}
		else if(key == "spread_min")
			m_spreadMin = uvec::create(pair.second);
		else if(key == "spread_max")
			m_spreadMax = uvec::create(pair.second);
		else if(key == "velocity_min")
		{
			if(m_randomVelocity == nullptr)
				m_randomVelocity = std::unique_ptr<RandomVelocity>(new RandomVelocity());
			m_randomVelocity->minVelocity = uvec::create(pair.second);
		}
		else if(key == "velocity_max")
		{
			if(m_randomVelocity == nullptr)
				m_randomVelocity = std::unique_ptr<RandomVelocity>(new RandomVelocity());
			m_randomVelocity->maxVelocity = uvec::create(pair.second);
		}
	}
}
void CParticleInitializerInitialAngularVelocity::OnParticleCreated(CParticle &particle)
{
	if(m_randomVelocity != nullptr)
	{
		auto vel = Vector3(
			umath::random(m_randomVelocity->minVelocity.x,m_randomVelocity->maxVelocity.x),
			umath::random(m_randomVelocity->minVelocity.y,m_randomVelocity->maxVelocity.y),
			umath::random(m_randomVelocity->minVelocity.z,m_randomVelocity->maxVelocity.z)
		);
		particle.SetAngularVelocity(vel);
		return;
	}
	if(m_speed == 0.f)
		return;
	auto dir = glm::normalize(m_direction +uvec::get_random_spread(m_spreadMin,m_spreadMax));
	particle.SetAngularVelocity(dir *m_speed);
}
