/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/operators/c_particle_mod_gravity.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

extern DLLCLIENT CGame *c_game;

REGISTER_PARTICLE_OPERATOR(gravity, CParticleOperatorGravity);

void CParticleOperatorGravity::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorWorldBase::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		StringToLower(key);
		if(key == "effective_scale")
			m_gravityScale = util::to_float(it->second);
		else if(key == "acceleration") {
			m_bUseCustomGravityForce = true;
			m_gravityForce = uvec::create(it->second);
		}
	}
}
void CParticleOperatorGravity::Simulate(double tDelta)
{
	CParticleOperatorWorldBase::Simulate(tDelta);
	if(m_bUseCustomGravityForce == false)
		return;
	m_dtGravity = GetParticleSystem().DirectionToParticleSpace(m_gravityForce * static_cast<float>(tDelta), ShouldRotateWithEmitter());
}
void CParticleOperatorGravity::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperatorWorldBase::Simulate(particle, tDelta, strength);
	if(m_bUseCustomGravityForce) {
		particle.SetVelocity(particle.GetVelocity() + m_dtGravity);
		return;
	}
	auto &gravity = c_game->GetGravity();
	auto &oldVel = particle.GetVelocity();
	particle.SetVelocity(oldVel + (m_bUseCustomGravityForce ? m_gravityForce : gravity) * m_gravityScale * static_cast<float>(tDelta));
}
