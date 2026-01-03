// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.initializer_initial_animation_frame;

import :client_state;

void pragma::pts::CParticleInitializerInitialAnimationFrame::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		string::to_lower(key);
		if(key == "frame_min")
			m_minFrame = util::to_float(it->second);
		else if(key == "frame_max")
			m_maxFrame = util::to_float(it->second);
	}
}
void pragma::pts::CParticleInitializerInitialAnimationFrame::OnParticleCreated(CParticle &particle)
{
	CParticleInitializer::OnParticleCreated(particle);
	particle.SetFrameOffset(math::random(m_minFrame, m_maxFrame));
}
