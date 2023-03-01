/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_MOD_COLOR_FADE_H__
#define __C_PARTICLE_MOD_COLOR_FADE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_gradual_fade.hpp"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_random_color.hpp"

class DLLCLIENT CParticleOperatorColorFade : public CParticleOperator, public CParticleModifierComponentGradualFade {
  public:
	CParticleOperatorColorFade() = default;
	virtual void Simulate(CParticle &particle, double, float strength) override;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
  private:
	CParticleModifierComponentRandomColor m_colorStart;
	CParticleModifierComponentRandomColor m_colorEnd;
	std::unique_ptr<std::vector<Color>> m_particleStartColors = nullptr;
};

#endif
