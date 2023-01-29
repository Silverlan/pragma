/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_angular_acceleration.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(angular_acceleration, CParticleOperatorAngularAcceleration);

void CParticleOperatorAngularAcceleration::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "acceleration")
			m_vAcceleration = uvec::create(pair.second);
	}
}
void CParticleOperatorAngularAcceleration::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	particle.SetAngularVelocity(particle.GetAngularVelocity() + m_vAcceleration * static_cast<float>(tDelta));
}
