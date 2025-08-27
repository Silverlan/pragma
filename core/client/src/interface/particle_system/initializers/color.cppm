// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

export module pragma.client.particle_system:initializer_color;

import :modifier_random_color;

export class DLLCLIENT CParticleInitializerColor : public CParticleInitializer, public CParticleModifierComponentRandomColor {
  public:
	CParticleInitializerColor() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
};

REGISTER_PARTICLE_INITIALIZER(color_random, CParticleInitializerColor);

void CParticleInitializerColor::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	CParticleModifierComponentRandomColor::Initialize("", values);
}
void CParticleInitializerColor::OnParticleCreated(CParticle &particle) { particle.SetColor(GetValue(particle)); }

