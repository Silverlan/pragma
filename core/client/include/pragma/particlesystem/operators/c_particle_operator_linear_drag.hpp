/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_OPERATOR_LINEAR_DRAG_HPP__
#define __C_PARTICLE_OPERATOR_LINEAR_DRAG_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorLinearDrag : public CParticleOperator {
  public:
	CParticleOperatorLinearDrag() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	virtual void Simulate(double tDelta) override;
  private:
	float m_fAmount = 1.f;
	float m_fTickDrag = 1.f;
};

#endif
