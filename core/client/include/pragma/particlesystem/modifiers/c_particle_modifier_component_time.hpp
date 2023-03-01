/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_MODIFIER_COMPONENT_TIME_HPP__
#define __C_PARTICLE_MODIFIER_COMPONENT_TIME_HPP__

#include "pragma/clientdefinitions.h"
#include <string>
#include <unordered_map>
#include <mathutil/umath_random.hpp>
#include <pragma/math/util_random.hpp>

class CParticle;
class DLLCLIENT CParticleModifierComponentTime {
  protected:
	CParticleModifierComponentTime() = default;
	void Initialize(const std::string &prefix, const std::unordered_map<std::string, std::string> &values);
	float GetTime(float t, CParticle &p) const;
  private:
	bool m_bLifetimeFraction = false;
};

#endif
