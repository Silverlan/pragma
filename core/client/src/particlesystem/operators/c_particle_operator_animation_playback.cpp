// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_animation_playback.hpp"
#include <mathutil/umath.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(animation_playback, CParticleOperatorAnimationPlayback);

void CParticleOperatorAnimationPlayback::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "playbackspeed")
			m_playbackSpeed = util::to_float(pair.second);
	}
}
void CParticleOperatorAnimationPlayback::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	particle.SetFrameOffset(fmodf(particle.GetFrameOffset() + tDelta * m_playbackSpeed, 1.f));
}
