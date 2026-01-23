// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.initializer_color;

export import :particle_system.modifier;
export import :particle_system.modifier_random_color;

export namespace pragma::pts {
	class DLLCLIENT CParticleInitializerColor : public CParticleInitializer, public CParticleModifierComponentRandomColor {
	  public:
		CParticleInitializerColor() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	};
}

void pragma::pts::CParticleInitializerColor::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleInitializer::Initialize(pSystem, values);
	CParticleModifierComponentRandomColor::Initialize("", values);
}
void pragma::pts::CParticleInitializerColor::OnParticleCreated(CParticle &particle) { particle.SetColor(GetValue(particle)); }
