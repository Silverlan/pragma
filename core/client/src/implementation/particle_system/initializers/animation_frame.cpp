// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

module pragma.client.particle_system;

import :initializer_initial_animation_frame;

import pragma.client.client_state;

REGISTER_PARTICLE_INITIALIZER(initial_animation_frame, CParticleInitializerInitialAnimationFrame);

void CParticleInitializerInitialAnimationFrame::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if(key == "frame_min")
			m_minFrame = util::to_float(it->second);
		else if(key == "frame_max")
			m_maxFrame = util::to_float(it->second);
	}
}
void CParticleInitializerInitialAnimationFrame::OnParticleCreated(CParticle &particle)
{
	CParticleInitializer::OnParticleCreated(particle);
	particle.SetFrameOffset(umath::random(m_minFrame, m_maxFrame));
}
