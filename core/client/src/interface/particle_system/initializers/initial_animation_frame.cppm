// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:particle_system.initializer_initial_animation_frame;

import :particle_system.modifier;

export class DLLCLIENT CParticleInitializerInitialAnimationFrame : public CParticleInitializer {
  private:
	float m_minFrame = 0.f;
	float m_maxFrame = 1.f;
  public:
	CParticleInitializerInitialAnimationFrame() = default;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
};
