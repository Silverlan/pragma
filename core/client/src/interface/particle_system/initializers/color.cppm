// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"

export module pragma.client:particle_system.initializer_color;

export import :particle_system.modifier;
export import :particle_system.modifier_random_color;

export class DLLCLIENT CParticleInitializerColor : public CParticleInitializer, public CParticleModifierComponentRandomColor {
  public:
	CParticleInitializerColor() = default;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
};

void CParticleInitializerColor::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	CParticleModifierComponentRandomColor::Initialize("", values);
}
void CParticleInitializerColor::OnParticleCreated(CParticle &particle) { particle.SetColor(GetValue(particle)); }
