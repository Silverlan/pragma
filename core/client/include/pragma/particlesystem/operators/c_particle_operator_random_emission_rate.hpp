#ifndef __C_PARTICLE_OPERATOR_RANDOM_EMISSION_RATE_HPP__
#define __C_PARTICLE_OPERATOR_RANDOM_EMISSION_RATE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorRandomEmissionRate
	: public CParticleOperator
{
public:
	CParticleOperatorRandomEmissionRate(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Simulate(double tDelta) override;
	virtual void Initialize() override;
private:
	float GetInterval() const;
	void Reset();
	float m_fMinimum = 0.07f;
	float m_fMaximum = 0.2f;
	float m_fRemaining = 0.f;
};

#endif
