/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_MOD_INITIAL_ANGULAR_VELOCITY_HPP__
#define __C_PARTICLE_MOD_INITIAL_ANGULAR_VELOCITY_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleInitializerInitialAngularVelocity
	: public CParticleInitializer
{
private:
	Vector3 m_direction = {};
	float m_speed = 0.f;
	Vector3 m_spreadMin = {};
	Vector3 m_spreadMax = {};

	struct RandomVelocity
	{
		Vector3 minVelocity = {};
		Vector3 maxVelocity = {};
	};
	std::unique_ptr<RandomVelocity> m_randomVelocity = nullptr;
public:
	CParticleInitializerInitialAngularVelocity()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
};

#endif