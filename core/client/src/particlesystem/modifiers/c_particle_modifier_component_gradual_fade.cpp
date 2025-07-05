// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/particlesystem/c_particle.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_gradual_fade.hpp"

void CParticleModifierComponentGradualFade::Initialize(const std::unordered_map<std::string, std::string> &values)
{
	CParticleModifierComponentTime::Initialize("fade", values);
	CParticleModifierComponentEase::Initialize(values);
	m_fStart.Initialize("fade_start", values);
	m_fEnd.Initialize("fade_end", values);
}

float CParticleModifierComponentGradualFade::GetStartTime(CParticle &p) const
{
	auto tStart = m_fStart.GetValue(p);
	return GetTime(tStart, p);
}
float CParticleModifierComponentGradualFade::GetEndTime(CParticle &p) const
{
	auto tEnd = m_fEnd.GetValue(p);
	return GetTime(tEnd, p);
}
bool CParticleModifierComponentGradualFade::GetFadeFraction(CParticle &p, float &outFraction) const
{
	auto tStart = GetStartTime(p);
	auto t = p.GetTimeAlive();
	if(t < tStart) {
		outFraction = 0.f;
		return false;
	}
	auto tEnd = GetEndTime(p);
	auto tDelta = tEnd - tStart;
	outFraction = (tDelta != 0.f) ? umath::clamp((t - tStart) / (tEnd - tStart), 0.f, 1.f) : 0.f;
	return true;
}
float CParticleModifierComponentGradualFade::GetFadeFraction(CParticle &p) const
{
	auto fraction = 0.f;
	GetFadeFraction(p, fraction);
	return fraction;
}
bool CParticleModifierComponentGradualFade::GetEasedFadeFraction(CParticle &p, float &outFraction) const
{
	if(GetFadeFraction(p, outFraction) == false)
		return false;
	outFraction = Ease(outFraction);
	return true;
}
float CParticleModifierComponentGradualFade::GetEasedFadeFraction(CParticle &p) const
{
	auto fraction = 0.f;
	GetEasedFadeFraction(p, fraction);
	return fraction;
}
