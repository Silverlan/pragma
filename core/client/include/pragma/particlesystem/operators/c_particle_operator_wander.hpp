#ifndef __C_PARTICLE_OPERATOR_WANDER_HPP__
#define __C_PARTICLE_OPERATOR_WANDER_HPP__

#include "pragma/particlesystem/operators/c_particle_operator_world_base.hpp"

class DLLCLIENT CParticleOperatorWander
	: public CParticleOperatorWorldBase
{
public:
	CParticleOperatorWander(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Simulate(CParticle &particle,double tDelta) override;
	virtual void Simulate(double tDelta) override;
	virtual void Initialize(CParticle &particle) override;
protected:
	std::vector<int32_t> m_hashCodes;
	float m_fFrequency = 2.f;
	float m_fStrength = 0.05f;

	float m_dtTime = 0.f;
	float m_dtStrength = 0.f;
};

#endif
