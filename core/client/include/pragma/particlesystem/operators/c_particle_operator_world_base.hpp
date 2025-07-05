// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_OPERATOR_WORLD_BASE_HPP__
#define __C_PARTICLE_OPERATOR_WORLD_BASE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorWorldBase : public CParticleOperator {
  public:
	bool ShouldRotateWithEmitter() const;
  protected:
	CParticleOperatorWorldBase() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
  private:
	bool m_bRotateWithEmitter = false;
};

#endif
