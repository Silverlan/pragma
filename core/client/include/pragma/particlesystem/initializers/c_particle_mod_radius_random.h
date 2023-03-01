/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_MOD_RADIUS_RANDOM_H__
#define __C_PARTICLE_MOD_RADIUS_RANDOM_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleInitializerRadiusRandomBase : public CParticleInitializer {
  public:
	CParticleInitializerRadiusRandomBase(const std::string &identifier);
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
	float GetMinRadius() const;
	float GetMaxRadius() const;
  protected:
	virtual void ApplyRadius(CParticle &particle, float radius) = 0;
  private:
	float m_radiusMin = 0.f;
	float m_radiusMax = 0.f;
	std::string m_identifier;
};

////////////////////////////

class DLLCLIENT CParticleInitializerRadiusRandom : public CParticleInitializerRadiusRandomBase {
  public:
	CParticleInitializerRadiusRandom();
  protected:
	virtual void ApplyRadius(CParticle &particle, float radius) override;
};

////////////////////////////

class DLLCLIENT CParticleInitializerLengthRandom : public CParticleInitializerRadiusRandomBase {
  public:
	CParticleInitializerLengthRandom();
  protected:
	virtual void ApplyRadius(CParticle &particle, float radius) override;
};

#endif
