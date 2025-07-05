// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_color.hpp"

REGISTER_PARTICLE_INITIALIZER(color_random, CParticleInitializerColor);

void CParticleInitializerColor::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	CParticleModifierComponentRandomColor::Initialize("", values);
}
void CParticleInitializerColor::OnParticleCreated(CParticle &particle) { particle.SetColor(GetValue(particle)); }
