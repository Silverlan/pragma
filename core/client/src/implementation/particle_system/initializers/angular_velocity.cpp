// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.initializer_initial_angular_velocity;

import :client_state;

void pragma::pts::CParticleInitializerInitialAngularVelocity::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "velocity") {
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
		else if(key == "velocity_min") {
			if(m_randomVelocity == nullptr)
				m_randomVelocity = std::unique_ptr<RandomVelocity>(new RandomVelocity());
			m_randomVelocity->minVelocity = uvec::create(pair.second);
		}
		else if(key == "velocity_max") {
			if(m_randomVelocity == nullptr)
				m_randomVelocity = std::unique_ptr<RandomVelocity>(new RandomVelocity());
			m_randomVelocity->maxVelocity = uvec::create(pair.second);
		}
	}
}
void pragma::pts::CParticleInitializerInitialAngularVelocity::OnParticleCreated(CParticle &particle)
{
	if(m_randomVelocity != nullptr) {
		auto vel = Vector3(math::random(m_randomVelocity->minVelocity.x, m_randomVelocity->maxVelocity.x), math::random(m_randomVelocity->minVelocity.y, m_randomVelocity->maxVelocity.y), math::random(m_randomVelocity->minVelocity.z, m_randomVelocity->maxVelocity.z));
		particle.SetAngularVelocity(vel);
		return;
	}
	if(m_speed == 0.f)
		return;
	auto dir = glm::normalize(m_direction + uvec::get_random_spread(m_spreadMin, m_spreadMax));
	particle.SetAngularVelocity(dir * m_speed);
}
