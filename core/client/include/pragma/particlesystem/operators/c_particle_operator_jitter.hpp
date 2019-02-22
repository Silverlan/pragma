#ifndef __C_PARTICLE_OPERATOR_JITTER_HPP__
#define __C_PARTICLE_OPERATOR_JITTER_HPP__

#include "pragma/particlesystem/operators/c_particle_operator_wander.hpp"

class DLLCLIENT CParticleOperatorJitter
	: public CParticleOperatorWander
{
public:
	CParticleOperatorJitter(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Simulate(CParticle &particle,double tDelta) override;
};

#endif
