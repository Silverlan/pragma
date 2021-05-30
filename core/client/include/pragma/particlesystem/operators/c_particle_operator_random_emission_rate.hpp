/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_OPERATOR_RANDOM_EMISSION_RATE_HPP__
#define __C_PARTICLE_OPERATOR_RANDOM_EMISSION_RATE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorRandomEmissionRate
	: public CParticleOperator
{
public:
	CParticleOperatorRandomEmissionRate()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(double tDelta) override;
	virtual void OnParticleSystemStarted() override;
private:
	float GetInterval() const;
	void Reset();
	float m_fMinimum = 0.07f;
	float m_fMaximum = 0.2f;
	float m_fRemaining = 0.f;
};

#endif
