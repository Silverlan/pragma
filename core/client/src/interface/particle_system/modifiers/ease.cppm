// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <string>
#include <unordered_map>
#include <mathutil/umath.h>

export module pragma.client:particle_system.modifier_ease;

export import pragma.shared;

export class DLLCLIENT CParticleModifierComponentEase {
  protected:
	CParticleModifierComponentEase();
	void Initialize(const std::unordered_map<std::string, std::string> &values);

	float Ease(float t) const;
	float EaseIn(float t) const;
	float EaseOut(float t) const;
	float EaseInOut(float t) const;
  private:
	enum class EaseFunc : uint32_t { None = 0u, In, Out, InOut };
	umath::EaseType m_type;
	EaseFunc m_easeFunc = EaseFunc::InOut;
};

CParticleModifierComponentEase::CParticleModifierComponentEase() : m_type {umath::EaseType::Linear} {}
void CParticleModifierComponentEase::Initialize(const std::unordered_map<std::string, std::string> &values)
{
	auto bEaseIn = true;
	auto bEaseOut = true;
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "ease")
			m_type = static_cast<umath::EaseType>(util::to_int(it->second));
		else if(key == "ease_in")
			bEaseIn = util::to_boolean(it->second);
		else if(key == "ease_out")
			bEaseOut = util::to_boolean(it->second);
	}
	m_easeFunc = (bEaseIn && bEaseOut) ? EaseFunc::InOut : bEaseIn ? EaseFunc::In : bEaseOut ? EaseFunc::Out : EaseFunc::None;
}

float CParticleModifierComponentEase::Ease(float t) const
{
	switch(m_easeFunc) {
	case EaseFunc::In:
		return EaseIn(t);
	case EaseFunc::Out:
		return EaseOut(t);
	case EaseFunc::InOut:
		return EaseInOut(t);
	default:
		return t;
	}
}
float CParticleModifierComponentEase::EaseIn(float t) const { return umath::ease_in(t, m_type); }
float CParticleModifierComponentEase::EaseOut(float t) const { return umath::ease_out(t, m_type); }
float CParticleModifierComponentEase::EaseInOut(float t) const { return umath::ease_in_out(t, m_type); }
