/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLE_MOD_GRAVITY_H__
#define __C_PARTICLE_MOD_GRAVITY_H__

#include "pragma/particlesystem/operators/c_particle_operator_world_base.hpp"

class DLLCLIENT CParticleOperatorGravity
	: public CParticleOperatorWorldBase
{
public:
	CParticleOperatorGravity()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double tDelta,float strength) override;
	virtual void Simulate(double tDelta) override;
protected:
	float m_gravityScale = 1.f;
	Vector3 m_gravityForce = {0.f,-1.f,0.f};
	bool m_bUseCustomGravityForce = false;

	Vector3 m_dtGravity = {};
};

#endif
