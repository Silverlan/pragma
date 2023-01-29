/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/c_particle.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_time.hpp"

void CParticleModifierComponentTime::Initialize(const std::string &prefix, const std::unordered_map<std::string, std::string> &values)
{
	std::string identifier = "lifetime_fraction";
	if(prefix.empty() == false)
		identifier = prefix + "_" + identifier;
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == identifier)
			m_bLifetimeFraction = util::to_boolean(it->second);
	}
}
float CParticleModifierComponentTime::GetTime(float t, CParticle &p) const
{
	if(m_bLifetimeFraction == false) {
		if(t < 0.f)
			t += p.GetLifeSpan();
		return t;
	}
	if(t < 0.f)
		t += 1.f;
	return t * p.GetLifeSpan();
}
