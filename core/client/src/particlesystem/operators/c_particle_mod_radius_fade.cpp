/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_mod_radius_fade.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <pragma/math/util_random.hpp>
#include <mathutil/umath_random.hpp>

REGISTER_PARTICLE_OPERATOR(radius_fade, CParticleOperatorRadiusFade);
REGISTER_PARTICLE_OPERATOR(length_fade, CParticleOperatorLengthFade);

CParticleOperatorRadiusFadeBase::CParticleOperatorRadiusFadeBase(const std::string &identifier) : CParticleOperator {}, m_identifier {identifier} {}
void CParticleOperatorRadiusFadeBase::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	CParticleModifierComponentGradualFade::Initialize(values);
	m_fRadiusStart.Initialize(m_identifier + "_start", values);
	m_fRadiusEnd.Initialize(m_identifier + "_end", values);

	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		StringToLower(key);
		if(key == m_identifier) // Alternative to "radius_end"
			m_fRadiusEnd.SetRange(util::to_float(it->second));
	}

	// If no start radius has been specified, the previous known radius of the particle has to be used as start radius.
	// Since that radius cannot be known beforehand, we need to store it.
	if(m_fRadiusStart.IsSet() == false)
		m_particleStartRadiuses = std::make_unique<std::vector<float>>(pSystem.GetMaxParticleCount(), std::numeric_limits<float>::max());
}
void CParticleOperatorRadiusFadeBase::OnParticleCreated(CParticle &particle)
{
	if(m_particleStartRadiuses == nullptr)
		return;
	m_particleStartRadiuses->at(particle.GetIndex()) = std::numeric_limits<float>::max();
}
void CParticleOperatorRadiusFadeBase::Simulate(CParticle &particle, double, float strength)
{
	auto tFade = 0.f;
	if(GetEasedFadeFraction(particle, tFade) == false)
		return;
	auto radiusStart = 0.f;
	if(m_particleStartRadiuses != nullptr) {
		// Use last known particle radius
		auto &ptRadiusStart = m_particleStartRadiuses->at(particle.GetIndex());
		if(ptRadiusStart == std::numeric_limits<float>::max())
			ptRadiusStart = particle.GetRadius();
		radiusStart = ptRadiusStart;
	}
	else
		radiusStart = m_fRadiusStart.GetValue(particle);
	auto radiusEnd = m_fRadiusEnd.GetValue(particle);
	auto radius = radiusStart + (radiusEnd - radiusStart) * tFade;
	ApplyRadius(particle, radius);
}

////////////////////////////

CParticleOperatorRadiusFade::CParticleOperatorRadiusFade() : CParticleOperatorRadiusFadeBase("radius") {}
void CParticleOperatorRadiusFade::ApplyRadius(CParticle &particle, float radius) const { particle.SetRadius(radius); }

////////////////////////////

CParticleOperatorLengthFade::CParticleOperatorLengthFade() : CParticleOperatorRadiusFadeBase("length") {}
void CParticleOperatorLengthFade::ApplyRadius(CParticle &particle, float radius) const { particle.SetLength(radius); }
