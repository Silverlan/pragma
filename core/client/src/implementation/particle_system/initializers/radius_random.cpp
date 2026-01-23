// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.initializer_radius_random;

pragma::pts::CParticleInitializerRadiusRandomBase::CParticleInitializerRadiusRandomBase(const std::string &identifier) : m_identifier {identifier} {}
void pragma::pts::CParticleInitializerRadiusRandomBase::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		string::to_lower(key);
		if(key == m_identifier + "_min")
			m_radiusMin = util::to_float(it->second);
		else if(key == m_identifier + "_max")
			m_radiusMax = util::to_float(it->second);
	}
}
void pragma::pts::CParticleInitializerRadiusRandomBase::OnParticleCreated(CParticle &particle) { ApplyRadius(particle, math::random(m_radiusMin, m_radiusMax)); }
float pragma::pts::CParticleInitializerRadiusRandomBase::GetMinRadius() const { return m_radiusMin; }
float pragma::pts::CParticleInitializerRadiusRandomBase::GetMaxRadius() const { return m_radiusMax; }

////////////////////////////

pragma::pts::CParticleInitializerRadiusRandom::CParticleInitializerRadiusRandom() : CParticleInitializerRadiusRandomBase("radius") {}
void pragma::pts::CParticleInitializerRadiusRandom::ApplyRadius(CParticle &particle, float radius) { particle.SetRadius(radius); }

////////////////////////////

pragma::pts::CParticleInitializerLengthRandom::CParticleInitializerLengthRandom() : CParticleInitializerRadiusRandomBase("length") {}
void pragma::pts::CParticleInitializerLengthRandom::ApplyRadius(CParticle &particle, float radius) { particle.SetLength(radius); }
