/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_OPERATOR_JITTER_HPP__
#define __C_PARTICLE_OPERATOR_JITTER_HPP__

#include "pragma/particlesystem/operators/c_particle_operator_wander.hpp"

class DLLCLIENT CParticleOperatorJitter
	: public CParticleOperatorWander
{
public:
	CParticleOperatorJitter()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double tDelta,float strength) override;
};

#endif
