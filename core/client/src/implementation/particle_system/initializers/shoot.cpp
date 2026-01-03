// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.initializer_shoot;

void pragma::pts::CParticleInitializerShootCone::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	m_fMinAngle.Initialize("angle_min", values);
	m_fMaxAngle.Initialize("angle_max", values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "direction")
			m_vDirection = uvec::create(pair.second);
	}
}
void pragma::pts::CParticleInitializerShootCone::OnParticleCreated(CParticle &particle)
{
	auto rot = glm::gtx::rotation(Vector3(0.f, 1.f, 0.f), m_vDirection);

	// pick an angle off the vertical based on the surface area distribution
	auto cosa = math::random(static_cast<float>(math::cos(math::deg_to_rad(m_fMinAngle.GetValue(particle)))), static_cast<float>(math::cos(math::deg_to_rad(m_fMaxAngle.GetValue(particle)))));
	auto sina = math::sqrt(1.f - math::pow2(cosa));
	auto theta = math::random(0.f, math::pi * 2.f);

	// set, transform
	auto vel = Vector3(math::cos(theta) * sina, cosa, -math::sin(theta) * sina);
	uvec::rotate(&vel, rot);
	particle.SetVelocity(vel);
}

/////////////////////////

void pragma::pts::CParticleInitializerShootOutward::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "bias")
			m_vBias = uvec::create(pair.second);
	}
}
void pragma::pts::CParticleInitializerShootOutward::OnParticleCreated(CParticle &particle)
{
	auto vel = GetParticleSystem().PointToParticleSpace(Vector3 {}, true);
	vel = particle.GetPosition() - vel;
	auto length = uvec::length(vel);
	if(length > 0.001f) // use the vector from origin to particle
		vel /= length;
	else // pick a random direction
		vel = uvec::create_random_unit_vector();
	vel += m_vBias;
	uvec::normalize(&vel);
	particle.SetVelocity(vel);
}
