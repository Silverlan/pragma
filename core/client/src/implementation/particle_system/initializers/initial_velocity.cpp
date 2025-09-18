// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

module pragma.client.particle_system;

import :initializer_initial_velocity;

import pragma.client.client_state;

REGISTER_PARTICLE_INITIALIZER(initial_velocity, CParticleInitializerInitialVelocity);

void CParticleInitializerInitialVelocity::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if(key == "velocity") {
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
		else if(key == "velocity_min")
			m_velocityMin = uvec::create(it->second);
		else if(key == "velocity_max")
			m_velocityMax = uvec::create(it->second);
	}
}
void CParticleInitializerInitialVelocity::OnParticleCreated(CParticle &particle)
{
	auto dir = m_direction;
	if(uvec::length_sqr(m_spreadMax - m_spreadMin) > 0.001f) {
		dir += uvec::get_random_spread(m_spreadMin, m_spreadMax);
		uvec::normalize(&dir);
	}
	auto vel = dir * m_speed;
	vel += uvec::get_random_spread(m_velocityMin, m_velocityMax);
	particle.SetVelocity(vel);
}
float CParticleInitializerInitialVelocity::GetSpeed() const { return m_speed; }
