/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_OPERATOR_WANDER_HPP__
#define __C_PARTICLE_OPERATOR_WANDER_HPP__

#include "pragma/particlesystem/operators/c_particle_operator_world_base.hpp"

class DLLCLIENT CParticleOperatorWander
	: public CParticleOperatorWorldBase
{
public:
	CParticleOperatorWander()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double tDelta,float strength) override;
	virtual void Simulate(double tDelta) override;
	virtual void OnParticleCreated(CParticle &particle) override;
protected:
	std::vector<int32_t> m_hashCodes;
	float m_fFrequency = 2.f;
	float m_fStrength = 0.05f;

	float m_dtTime = 0.f;
	float m_dtStrength = 0.f;
};

#endif
