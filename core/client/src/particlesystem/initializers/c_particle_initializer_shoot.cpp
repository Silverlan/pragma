#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_shoot.hpp"

REGISTER_PARTICLE_INITIALIZER(shoot_cone,CParticleInitializerShootCone);
REGISTER_PARTICLE_INITIALIZER(shoot_outward,CParticleInitializerShootOutward);

CParticleInitializerShootCone::CParticleInitializerShootCone(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleInitializer(pSystem,values),m_fMinAngle("angle_min",values),m_fMaxAngle("angle_max",values)
{
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "direction")
			m_vDirection = uvec::create(pair.second);
	}
}
void CParticleInitializerShootCone::Initialize(CParticle &particle)
{
	auto rot = glm::rotation(Vector3(0.f,1.f,0.f),m_vDirection);

	// pick an angle off the vertical based on the surface area distribution
	auto cosa = umath::random(
		static_cast<float>(umath::cos(umath::deg_to_rad(m_fMinAngle.GetValue(particle)))),
		static_cast<float>(umath::cos(umath::deg_to_rad(m_fMaxAngle.GetValue(particle))))
	);
	auto sina = umath::sqrt(1.f - umath::pow2(cosa));
	auto theta = umath::random(0.f,umath::pi *2.f);

	// set, transform
	auto vel = Vector3(umath::cos(theta) *sina,cosa,-umath::sin(theta) *sina);
	uvec::rotate(&vel,rot);
	particle.SetVelocity(vel);
}

/////////////////////////

CParticleInitializerShootOutward::CParticleInitializerShootOutward(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleInitializer(pSystem,values)
{
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "bias")
			m_vBias = uvec::create(pair.second);
	}
}
void CParticleInitializerShootOutward::Initialize(CParticle &particle)
{
	auto vel = GetParticleSystem().PointToParticleSpace(Vector3{},true);
	vel = particle.GetPosition() -vel;
	auto length = uvec::length(vel);
	if(length > 0.001f) // use the vector from origin to particle
		vel /= length;
	else // pick a random direction
		vel = uvec::create_random_unit_vector();
	vel += m_vBias;
	uvec::normalize(&vel);
	particle.SetVelocity(vel);
}
