// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:particle_system.initializer_speed;

import :particle_system.modifier_random_variable;
export import :particle_system.modifier;

export namespace pragma::ecs {class CParticleSystemComponent;}
export class DLLCLIENT CParticleInitializerSpeed : public CParticleInitializer {
  public:
	CParticleInitializerSpeed() = default;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
  private:
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fSpeed;
};

void CParticleInitializerSpeed::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	m_fSpeed.Initialize("speed", values);
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "speed_min")
			m_fSpeed.SetMin(util::to_float(pair.second));
		else if(key == "speed_max")
			m_fSpeed.SetMax(util::to_float(pair.second));
	}
}
void CParticleInitializerSpeed::OnParticleCreated(CParticle &particle)
{
	auto vel = particle.GetVelocity();
	auto l = uvec::length(vel);
	if(l <= 0.0001f)
		return;
	vel /= l;
	particle.SetVelocity(vel * m_fSpeed.GetValue(particle));
}
