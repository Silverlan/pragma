/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_random_emission_rate.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(emission_rate_random,CParticleOperatorRandomEmissionRate);

void CParticleOperatorRandomEmissionRate::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleOperator::Initialize(pSystem,values);
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "minimum")
			m_fMinimum = util::to_float(pair.second);
		else if(key == "maximum")
			m_fMaximum = util::to_float(pair.second);
	}
}
void CParticleOperatorRandomEmissionRate::OnParticleSystemStarted()
{
	CParticleOperator::OnParticleSystemStarted();
	Reset();
	Simulate(0.f);
}
void CParticleOperatorRandomEmissionRate::Reset() {m_fRemaining = GetInterval();}
float CParticleOperatorRandomEmissionRate::GetInterval() const {return umath::max(0.f,umath::random(m_fMinimum,m_fMaximum));}
void CParticleOperatorRandomEmissionRate::Simulate(double tDelta)
{
	CParticleOperator::Simulate(tDelta);

	auto &ps = GetParticleSystem();
	if((m_fRemaining -= tDelta) > 0.f)
	{
		ps.SetNextParticleEmissionCount(0u);
		return;
	}
	auto maximum = ps.GetMaxParticleCount();
	auto count = umath::min(1u,maximum);
	while(count < maximum && (m_fRemaining += GetInterval()) < 0.f)
		++count;
	ps.SetNextParticleEmissionCount(count);
}

