/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_linear_drag.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(linear_drag,CParticleOperatorLinearDrag);

void CParticleOperatorLinearDrag::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleOperator::Initialize(pSystem,values);
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "drag")
			m_fAmount = util::to_float(it->second);
	}
}
void CParticleOperatorLinearDrag::Simulate(double tDelta)
{
	CParticleOperator::Simulate(tDelta);
	m_fTickDrag = umath::max(0.f,1.f -m_fAmount *static_cast<float>(tDelta));
}
void CParticleOperatorLinearDrag::Simulate(CParticle &particle,double tDelta,float strength)
{
	CParticleOperator::Simulate(particle,tDelta,strength);
	particle.SetVelocity(particle.GetVelocity() *m_fTickDrag);
}
