#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_angular_acceleration.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(angular_acceleration,CParticleOperatorAngularAcceleration);

void CParticleOperatorAngularAcceleration::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleOperator::Initialize(pSystem,values);
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "acceleration")
			m_vAcceleration = uvec::create(pair.second);
	}
}
void CParticleOperatorAngularAcceleration::Simulate(CParticle &particle,double tDelta)
{
	CParticleOperator::Simulate(particle,tDelta);
	particle.SetAngularVelocity(particle.GetAngularVelocity() +m_vAcceleration *static_cast<float>(tDelta));
}
