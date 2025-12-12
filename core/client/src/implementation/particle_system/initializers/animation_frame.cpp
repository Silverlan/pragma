// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :particle_system.initializer_initial_animation_frame;

import :client_state;

void pragma::pts::CParticleInitializerInitialAnimationFrame::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		pragma::string::to_lower(key);
		if(key == "frame_min")
			m_minFrame = pragma::util::to_float(it->second);
		else if(key == "frame_max")
			m_maxFrame = pragma::util::to_float(it->second);
	}
}
void pragma::pts::CParticleInitializerInitialAnimationFrame::OnParticleCreated(pragma::pts::CParticle &particle)
{
	CParticleInitializer::OnParticleCreated(particle);
	particle.SetFrameOffset(pragma::math::random(m_minFrame, m_maxFrame));
}
