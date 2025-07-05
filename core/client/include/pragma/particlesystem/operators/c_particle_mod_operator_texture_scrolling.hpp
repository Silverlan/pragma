// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_MOD_INITIAL_ANIMATION_FRAME_HPP__
#define __C_PARTICLE_MOD_INITIAL_ANIMATION_FRAME_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorTextureScrolling : public CParticleOperator {
  public:
	CParticleOperatorTextureScrolling() = default;
	virtual void Simulate(CParticle &particle, double, float strength) override;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
  private:
	void SetFrameOffset(CParticle &particle, Vector2 uv);
	float m_fHorizontalSpeed = 0.f;
	float m_fVerticalSpeed = 0.f;
};

#endif
