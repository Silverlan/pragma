/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLE_OPERATOR_WIND_HPP__
#define __C_PARTICLE_OPERATOR_WIND_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorWind
	: public CParticleOperator
{
public:
	CParticleOperatorWind()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double tDelta) override;
	virtual void Simulate(double tDelta) override;
private:
	bool m_bRotateWithEmitter = false;
	float m_fStrength = 2.f;
	Vector3 m_vDirection = {1.f,0.f,0.f};
	Vector3 m_vDelta = {};
};

#endif
