/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_cylindrical_vortex.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(cylindrical_vortex, CParticleOperatorCylindricalVortex);

void CParticleOperatorCylindricalVortex::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorWorldBase::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "axis")
			m_vAxis = uvec::get_normal(uvec::create(it->second));
		else if(key == "strength")
			m_fStrength = util::to_float(it->second);
		else if(key == "divergence")
			m_fDivergence = util::to_float(it->second);
	}
}
void CParticleOperatorCylindricalVortex::Simulate(double tDelta)
{
	CParticleOperatorWorldBase::Simulate(tDelta);
	m_dtStrength = m_fStrength * tDelta;

	// transform the origin and axis into particle space
	auto &ps = GetParticleSystem();
	m_dtOrigin = ps.PointToParticleSpace(Vector3 {}, true);
	m_dtAxis = ps.DirectionToParticleSpace(m_vAxis, ShouldRotateWithEmitter());

	// find divergence rotation
	m_dtRotation = uquat::create(m_dtAxis, -m_fDivergence);
}
void CParticleOperatorCylindricalVortex::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperatorWorldBase::Simulate(particle, tDelta, strength);
	// cross product of vortex axis and relative position is direction
	auto v = uvec::cross(m_dtAxis, particle.GetPosition() - m_dtOrigin);
	auto l = uvec::length(v);
	const auto EPSILON = 0.0001f;
	if(l < EPSILON)
		return; // particle is on the axis
	// normalize direction, scale by delta, rotate, add to velocity
	v *= m_dtStrength / l;
	uvec::rotate(&v, m_dtRotation);
	particle.SetVelocity(particle.GetVelocity() + v);
}
