// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <string>
#include <unordered_map>
#include <mathutil/umath_random.hpp>

export module pragma.client:particle_system.modifier_time;

import :particle_system.particle;

export class DLLCLIENT CParticleModifierComponentTime {
  protected:
	CParticleModifierComponentTime() = default;
	void Initialize(const std::string &prefix, const std::unordered_map<std::string, std::string> &values);
	float GetTime(float t, CParticle &p) const;
  private:
	bool m_bLifetimeFraction = false;
};

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
