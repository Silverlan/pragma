#ifndef __C_PARTICLE_OPERATOR_ANGULAR_ACCELERATION_HPP__
#define __C_PARTICLE_OPERATOR_ANGULAR_ACCELERATION_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorAngularAcceleration
	: public CParticleOperator
{
public:
	CParticleOperatorAngularAcceleration()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double tDelta) override;
private:
	Vector3 m_vAcceleration = {};
};

#endif
