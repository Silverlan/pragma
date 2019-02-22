#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_mod_radius_fade.h"
#include <pragma/math/util_random.hpp>
#include <mathutil/umath_random.hpp>

REGISTER_PARTICLE_OPERATOR(radius_fade,CParticleOperatorRadiusFade);
REGISTER_PARTICLE_OPERATOR(length_fade,CParticleOperatorLengthFade);

CParticleOperatorRadiusFadeBase::CParticleOperatorRadiusFadeBase(const std::string &identifier,pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperator(pSystem,values),CParticleModifierComponentGradualFade(values),
	m_fRadiusStart(identifier +"_start",values),m_fRadiusEnd(identifier +"_end",values)
{
	for(auto it=values.begin();it!=values.end();it++)
	{
		std::string key = it->first;
		StringToLower(key);
		if(key == identifier) // Alternative to "radius_end"
			m_fRadiusEnd.SetRange(util::to_float(it->second));
	}

	// If no start radius has been specified, the previous known radius of the particle has to be used as start radius.
	// Since that radius cannot be known beforehand, we need to store it.
	if(m_fRadiusStart.IsSet() == false)
		m_particleStartRadiuses = std::make_unique<std::vector<float>>(pSystem.GetMaxParticleCount(),std::numeric_limits<float>::max());
}
void CParticleOperatorRadiusFadeBase::Initialize(CParticle &particle)
{
	if(m_particleStartRadiuses == nullptr)
		return;
	m_particleStartRadiuses->at(particle.GetIndex()) = std::numeric_limits<float>::max();
}
void CParticleOperatorRadiusFadeBase::Simulate(CParticle &particle,double)
{
	auto tFade = 0.f;
	if(GetEasedFadeFraction(particle,tFade) == false)
		return;
	auto radiusStart = 0.f;
	if(m_particleStartRadiuses != nullptr)
	{
		// Use last known particle radius
		auto &ptRadiusStart = m_particleStartRadiuses->at(particle.GetIndex());
		if(ptRadiusStart == std::numeric_limits<float>::max())
			ptRadiusStart = particle.GetRadius();
		radiusStart = ptRadiusStart;
	}
	else
		radiusStart = m_fRadiusStart.GetValue(particle);
	auto radiusEnd = m_fRadiusEnd.GetValue(particle);
	auto radius = radiusStart +(radiusEnd -radiusStart) *tFade;
	ApplyRadius(particle,radius);
}

////////////////////////////

CParticleOperatorRadiusFade::CParticleOperatorRadiusFade(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperatorRadiusFadeBase("radius",pSystem,values)
{}
void CParticleOperatorRadiusFade::ApplyRadius(CParticle &particle,float radius) const {particle.SetRadius(radius);}

////////////////////////////

CParticleOperatorLengthFade::CParticleOperatorLengthFade(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperatorRadiusFadeBase("length",pSystem,values)
{}
void CParticleOperatorLengthFade::ApplyRadius(CParticle &particle,float radius) const {particle.SetLength(radius);}

