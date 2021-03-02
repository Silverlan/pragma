/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_speed.hpp"

REGISTER_PARTICLE_INITIALIZER(speed,CParticleInitializerSpeed);

void CParticleInitializerSpeed::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleInitializer::Initialize(pSystem,values);
	m_fSpeed.Initialize("speed",values);
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "speed_min")
			m_fSpeed.SetMin(util::to_float(pair.second));
		else if(key == "speed_max")
			m_fSpeed.SetMax(util::to_float(pair.second));
	}
}
void CParticleInitializerSpeed::OnParticleCreated(CParticle &particle)
{
	auto vel = particle.GetVelocity();
	auto l = uvec::length(vel);
	if(l <= 0.0001f)
		return;
	vel /= l;
	particle.SetVelocity(vel *m_fSpeed.GetValue(particle));
}
