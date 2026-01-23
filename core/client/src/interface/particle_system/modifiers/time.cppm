// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.modifier_time;

export import :particle_system.particle;
export import pragma.util;

export namespace pragma::pts {
	class DLLCLIENT CParticleModifierComponentTime {
	  protected:
		CParticleModifierComponentTime() = default;
		void Initialize(const std::string &prefix, const std::unordered_map<std::string, std::string> &values);
		float GetTime(float t, CParticle &p) const;
	  private:
		bool m_bLifetimeFraction = false;
	};
}

void pragma::pts::CParticleModifierComponentTime::Initialize(const std::string &prefix, const std::unordered_map<std::string, std::string> &values)
{
	std::string identifier = "lifetime_fraction";
	if(prefix.empty() == false)
		identifier = prefix + "_" + identifier;
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == identifier)
			m_bLifetimeFraction = util::to_boolean(it->second);
	}
}
float pragma::pts::CParticleModifierComponentTime::GetTime(float t, CParticle &p) const
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
