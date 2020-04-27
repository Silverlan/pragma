/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLE_MOD_INITIAL_ANIMATION_FRAME_HPP__
#define __C_PARTICLE_MOD_INITIAL_ANIMATION_FRAME_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleInitializerInitialAnimationFrame
	: public CParticleInitializer
{
private:
	float m_minFrame = 0.f;
	float m_maxFrame = 0.f;
	bool m_bUseFraction = false;
public:
	CParticleInitializerInitialAnimationFrame()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
};

#endif
