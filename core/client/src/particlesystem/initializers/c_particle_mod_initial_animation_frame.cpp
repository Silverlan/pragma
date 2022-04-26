/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/initializers/c_particle_mod_initial_animation_frame.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_INITIALIZER(initial_animation_frame,CParticleInitializerInitialAnimationFrame);

void CParticleInitializerInitialAnimationFrame::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleInitializer::Initialize(pSystem,values);
	for(auto it=values.begin();it!=values.end();it++)
	{
		std::string key = it->first;
		std::transform(key.begin(),key.end(),key.begin(),::tolower);
		if(key == "frame_min")
			m_minFrame = util::to_float(it->second);
		else if(key == "frame_max")
			m_maxFrame = util::to_float(it->second);
	}
}
void CParticleInitializerInitialAnimationFrame::OnParticleCreated(CParticle &particle)
{
	CParticleInitializer::OnParticleCreated(particle);
	particle.SetFrameOffset(umath::random(m_minFrame,m_maxFrame));
}
