// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.initializer_position;

void pragma::pts::CParticleInitializerPositionRandomBox::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		string::to_lower(key);
		if(key == "min")
			m_min = uvec::create(it->second);
		else if(key == "max")
			m_max = uvec::create(it->second);
		else if(key == "origin")
			m_origin = uvec::create(it->second);
		else if(key == "on_sides")
			m_bOnSides = util::to_boolean(it->second);
	}
}
void pragma::pts::CParticleInitializerPositionRandomBox::OnParticleCreated(CParticle &particle)
{
	if(m_bOnSides == true) {
		// Find a random position on one of the sides of the box
		auto dir = uvec::create_random_unit_vector();
		float tMinRes, tMaxRes;
		if(math::intersection::line_aabb({}, dir, m_min, m_max, &tMinRes, &tMaxRes) != math::intersection::Result::Intersect)
			return;
		particle.SetPosition(m_origin + dir * tMaxRes);
		return;
	}
	Vector3 pos(math::random(m_min.x, m_max.x), math::random(m_min.y, m_max.y), math::random(m_min.z, m_max.z));
	particle.SetPosition(m_origin + particle.GetPosition() + pos);
}

//////////////////////////////

void pragma::pts::CParticleInitializerPositionRandomSphere::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		string::to_lower(key);
		if(key == "distance_min")
			m_distMin = util::to_float(it->second);
		else if(key == "distance_max")
			m_distMax = util::to_float(it->second);
		else if(key == "distance_bias")
			distBias = uvec::create(it->second);
		else if(key == "origin")
			m_origin = uvec::create(it->second);
	}
}
void pragma::pts::CParticleInitializerPositionRandomSphere::OnParticleCreated(CParticle &particle)
{
	float offset = m_distMax - m_distMin;
	float r = m_distMin + sqrtf(math::random(0.f, offset * offset));
	float x = math::random(0.f, 1.f);
	float y = math::random(0.f, 1.f);
	float theta = 2.f * CFloat(math::pi) * x;
	float phi = (1.f - sqrtf(y)) * CFloat(math::pi) / 2.f;
	x = r * cosf(theta) * cosf(phi);
	y = r * sinf(phi) * ((math::random(0, 1) == 1) ? 1 : -1); // TODO: Avoid the additional call to Math::Random
	float z = r * sinf(theta) * cosf(phi);

	Vector3 pos(x, y, z);
	float l = uvec::length(pos);
	if(l > 0.0001f)
		uvec::normalize(&pos);
	else
		pos = {};
	pos *= distBias;
	pos *= l;

	particle.SetPosition(m_origin + particle.GetPosition() + pos);
}

//////////////////////////////

void pragma::pts::CParticleInitializerPositionRandomCircle::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "axis")
			m_vAxis = uvec::create(pair.second);
		else if(key == "distance_min")
			m_fMinDist = util::to_float(pair.second);
		else if(key == "distance_max")
			m_fMaxDist = util::to_float(pair.second);
		else if(key == "origin")
			m_origin = uvec::create(pair.second);
	}
}
void pragma::pts::CParticleInitializerPositionRandomCircle::OnParticleCreated(CParticle &particle)
{
	auto offset = m_fMaxDist - m_fMinDist;
	auto r = m_fMinDist + sqrtf(math::random(0.f, offset * offset));
	auto ang = math::random(0.f, 1.f) * math::pi * 2.f;
	auto pos = Vector3(math::cos(ang) * r, 0.f, math::sin(ang) * r);
	auto rot = glm::gtx::rotation(Vector3(0.f, 1.f, 0.f), m_vAxis);
	uvec::rotate(&pos, rot);
	particle.SetPosition(m_origin + particle.GetPosition() + pos);
}
