#ifndef __C_PARTICLE_OPERATOR_QUADRATIC_DRAG_HPP__
#define __C_PARTICLE_OPERATOR_QUADRATIC_DRAG_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorQuadraticDrag
	: public CParticleOperator
{
public:
	CParticleOperatorQuadraticDrag()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double tDelta) override;
	virtual void Simulate(double tDelta) override;
private:
	float m_fAmount = 1.f;
	float m_fTickDrag = 1.f;
};

#endif
