/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_color.hpp"

REGISTER_PARTICLE_INITIALIZER(color_random,CParticleInitializerColor);

void CParticleInitializerColor::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleInitializer::Initialize(pSystem,values);
	CParticleModifierComponentRandomColor::Initialize("",values);
}
void CParticleInitializerColor::OnParticleCreated(CParticle &particle) {particle.SetColor(GetValue(particle));}
