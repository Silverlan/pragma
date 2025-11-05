// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :particle_system.initializer_initial_animation_frame;

import :client_state;

void CParticleInitializerInitialAnimationFrame::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
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
