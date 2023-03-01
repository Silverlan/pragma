/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_MODIFIER_COMPONENT_GRADUAL_FADE_VARIABLE_HPP__
#define __C_PARTICLE_MODIFIER_COMPONENT_GRADUAL_FADE_VARIABLE_HPP__

#include "pragma/particlesystem/modifiers/c_particle_modifier_component_ease.hpp"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_time.hpp"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_random_variable.hpp"

class DLLCLIENT CParticleModifierComponentGradualFade : public CParticleModifierComponentEase, public CParticleModifierComponentTime {
  protected:
	CParticleModifierComponentGradualFade() = default;
	void Initialize(const std::unordered_map<std::string, std::string> &values);

	float GetStartTime(CParticle &p) const;
	float GetEndTime(CParticle &p) const;
	// Returns a value in [0,1] representing the current fade position (0 = start, 1 = end)
	float GetFadeFraction(CParticle &p) const;
	bool GetFadeFraction(CParticle &p, float &outFraction) const;

	// Returns the eased fade fraction
	float GetEasedFadeFraction(CParticle &p) const;
	bool GetEasedFadeFraction(CParticle &p, float &outFraction) const;
  private:
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fStart;
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fEnd;
	// If false, the start and end time will be in seconds (starting at the particle creation time)
	// If true, the start and end time will be fractions of the particle's total lifetime
};

#endif
