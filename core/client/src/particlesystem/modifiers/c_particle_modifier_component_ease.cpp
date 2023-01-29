/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/c_particle.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_ease.hpp"
#include <mathutil/umath.h>
#include <pragma/math/util_easing.hpp>

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
