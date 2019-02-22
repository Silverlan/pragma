#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_jitter.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/math/util_noise.hpp>
#include <algorithm>

extern DLLCENGINE CEngine *c_engine;

REGISTER_PARTICLE_OPERATOR(jitter,CParticleOperatorJitter);

CParticleOperatorJitter::CParticleOperatorJitter(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperatorWander(pSystem,values)
{}
void CParticleOperatorJitter::Simulate(CParticle &particle,double tDelta)
{
	CParticleOperatorWorldBase::Simulate(particle,tDelta);

	// jitter is just like wander, except it directly influences the position
	auto pid = m_hashCodes.at(particle.GetIndex());
	auto time = m_dtTime +(pid &255) /256.f;
	particle.SetPosition(particle.GetPosition() +Vector3(
		util::noise::get_noise(time,pid) *m_dtStrength,
		util::noise::get_noise(time,pid +1) *m_dtStrength,
		util::noise::get_noise(time,pid +2) *m_dtStrength
	));
}
