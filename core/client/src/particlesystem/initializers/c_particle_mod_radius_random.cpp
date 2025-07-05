// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_mod_radius_random.h"
#include <mathutil/umath.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_INITIALIZER(radius_random, CParticleInitializerRadiusRandom);
REGISTER_PARTICLE_INITIALIZER(length_random, CParticleInitializerLengthRandom);

CParticleInitializerRadiusRandomBase::CParticleInitializerRadiusRandomBase(const std::string &identifier) : m_identifier {identifier} {}
void CParticleInitializerRadiusRandomBase::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if(key == m_identifier + "_min")
			m_radiusMin = util::to_float(it->second);
		else if(key == m_identifier + "_max")
			m_radiusMax = util::to_float(it->second);
	}
}
void CParticleInitializerRadiusRandomBase::OnParticleCreated(CParticle &particle) { ApplyRadius(particle, umath::random(m_radiusMin, m_radiusMax)); }
float CParticleInitializerRadiusRandomBase::GetMinRadius() const { return m_radiusMin; }
float CParticleInitializerRadiusRandomBase::GetMaxRadius() const { return m_radiusMax; }

////////////////////////////

CParticleInitializerRadiusRandom::CParticleInitializerRadiusRandom() : CParticleInitializerRadiusRandomBase("radius") {}
void CParticleInitializerRadiusRandom::ApplyRadius(CParticle &particle, float radius) { particle.SetRadius(radius); }

////////////////////////////

CParticleInitializerLengthRandom::CParticleInitializerLengthRandom() : CParticleInitializerRadiusRandomBase("length") {}
void CParticleInitializerLengthRandom::ApplyRadius(CParticle &particle, float radius) { particle.SetLength(radius); }
