// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.modifier_gradual_fade;

export import :particle_system.modifier_ease;
import :particle_system.modifier_random_variable;
export import :particle_system.modifier_time;

export namespace pragma::pts {
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
}

void pragma::pts::CParticleModifierComponentGradualFade::Initialize(const std::unordered_map<std::string, std::string> &values)
{
	CParticleModifierComponentTime::Initialize("fade", values);
	CParticleModifierComponentEase::Initialize(values);
	m_fStart.Initialize("fade_start", values);
	m_fEnd.Initialize("fade_end", values);
}

float pragma::pts::CParticleModifierComponentGradualFade::GetStartTime(CParticle &p) const
{
	auto tStart = m_fStart.GetValue(p);
	return GetTime(tStart, p);
}
float pragma::pts::CParticleModifierComponentGradualFade::GetEndTime(CParticle &p) const
{
	auto tEnd = m_fEnd.GetValue(p);
	return GetTime(tEnd, p);
}
bool pragma::pts::CParticleModifierComponentGradualFade::GetFadeFraction(CParticle &p, float &outFraction) const
{
	auto tStart = GetStartTime(p);
	auto t = p.GetTimeAlive();
	if(t < tStart) {
		outFraction = 0.f;
		return false;
	}
	auto tEnd = GetEndTime(p);
	auto tDelta = tEnd - tStart;
	outFraction = (tDelta != 0.f) ? math::clamp((t - tStart) / (tEnd - tStart), 0.f, 1.f) : 0.f;
	return true;
}
float pragma::pts::CParticleModifierComponentGradualFade::GetFadeFraction(CParticle &p) const
{
	auto fraction = 0.f;
	GetFadeFraction(p, fraction);
	return fraction;
}
bool pragma::pts::CParticleModifierComponentGradualFade::GetEasedFadeFraction(CParticle &p, float &outFraction) const
{
	if(GetFadeFraction(p, outFraction) == false)
		return false;
	outFraction = Ease(outFraction);
	return true;
}
float pragma::pts::CParticleModifierComponentGradualFade::GetEasedFadeFraction(CParticle &p) const
{
	auto fraction = 0.f;
	GetEasedFadeFraction(p, fraction);
	return fraction;
}
