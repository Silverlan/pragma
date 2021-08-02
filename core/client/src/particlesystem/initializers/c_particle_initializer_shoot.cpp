/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_shoot.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"

REGISTER_PARTICLE_INITIALIZER(shoot_cone,CParticleInitializerShootCone);
REGISTER_PARTICLE_INITIALIZER(shoot_outward,CParticleInitializerShootOutward);

void CParticleInitializerShootCone::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleInitializer::Initialize(pSystem,values);
	m_fMinAngle.Initialize("angle_min",values);
	m_fMaxAngle.Initialize("angle_max",values);
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "direction")
			m_vDirection = uvec::create(pair.second);
	}
}
void CParticleInitializerShootCone::OnParticleCreated(CParticle &particle)
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

void CParticleInitializerShootOutward::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleInitializer::Initialize(pSystem,values);
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "bias")
			m_vBias = uvec::create(pair.second);
	}
}
void CParticleInitializerShootOutward::OnParticleCreated(CParticle &particle)
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
