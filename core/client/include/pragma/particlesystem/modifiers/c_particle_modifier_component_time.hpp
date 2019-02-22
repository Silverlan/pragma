#ifndef __C_PARTICLE_MODIFIER_COMPONENT_TIME_HPP__
#define __C_PARTICLE_MODIFIER_COMPONENT_TIME_HPP__

#include "pragma/clientdefinitions.h"
#include <string>
#include <unordered_map>
#include <mathutil/umath_random.hpp>
#include <pragma/math/util_random.hpp>

class CParticle;
class DLLCLIENT CParticleModifierComponentTime
{
protected:
	CParticleModifierComponentTime(const std::string &prefix,const std::unordered_map<std::string,std::string> &values);
	float GetTime(float t,CParticle &p) const;
private:
	bool m_bLifetimeFraction = false;
};

#endif
