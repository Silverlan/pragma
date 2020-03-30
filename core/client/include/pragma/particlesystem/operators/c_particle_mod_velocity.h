#ifndef __C_PARTICLE_MOD_VELOCITY_H__
#define __C_PARTICLE_MOD_VELOCITY_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorVelocity
	: public CParticleOperator
{
private:
	Vector3 m_velocity = {};
public:
	CParticleOperatorVelocity()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double tDelta) override;
	float GetSpeed() const;
};

#endif
