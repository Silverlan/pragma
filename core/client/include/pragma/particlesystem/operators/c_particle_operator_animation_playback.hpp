// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_OPERATOR_ANIMATION_PLAYBACK_HPP__
#define __C_PARTICLE_OPERATOR_ANIMATION_PLAYBACK_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorAnimationPlayback : public CParticleOperator {
  public:
	CParticleOperatorAnimationPlayback() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
  private:
	float m_playbackSpeed = 1.f;
};

#endif
