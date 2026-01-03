// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_color_fade;

export import :entities.components.particle_system;
export import :particle_system.modifier;
import :particle_system.modifier_gradual_fade;
import :particle_system.modifier_random_color;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorColorFade : public CParticleOperator, public CParticleModifierComponentGradualFade {
	  public:
		CParticleOperatorColorFade() = default;
		virtual void Simulate(CParticle &particle, double, float strength) override;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  private:
		CParticleModifierComponentRandomColor m_colorStart;
		CParticleModifierComponentRandomColor m_colorEnd;
		std::unique_ptr<std::vector<Color>> m_particleStartColors = nullptr;
	};
}

void pragma::pts::CParticleOperatorColorFade::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	CParticleModifierComponentGradualFade::Initialize(values);
	m_colorStart.Initialize("start", values);
	m_colorEnd.Initialize("end", values);

	m_colorEnd.Initialize("", values); // Allows "color" as alternative to "color_end"
	                                   // If no start color has been specified, the previous known color of the particle has to be used as start color.
	                                   // Since that color cannot be known beforehand, we need to store it.
	if(m_colorStart.IsSet() == false)
		m_particleStartColors = std::make_unique<std::vector<Color>>(static_cast<ecs::CParticleSystemComponent &>(pSystem).GetMaxParticleCount(), Color(std::numeric_limits<int16_t>::max(), 0, 0, 0));
}
void pragma::pts::CParticleOperatorColorFade::OnParticleCreated(CParticle &particle)
{
	if(m_particleStartColors == nullptr)
		return;
	m_particleStartColors->at(particle.GetIndex()) = Color(std::numeric_limits<int16_t>::max(), 0, 0, 0);
}
void pragma::pts::CParticleOperatorColorFade::Simulate(CParticle &particle, double, float strength)
{
	auto tFade = 0.f;
	if(GetEasedFadeFraction(particle, tFade) == false)
		return;
	auto colorStart = Color {0, 0, 0, 0};
	auto componentFlags = CParticleModifierComponentRandomColor::ComponentFlags::None;
	if(m_particleStartColors != nullptr) {
		// Use last known particle color
		auto &ptColorStart = m_particleStartColors->at(particle.GetIndex());
		if(ptColorStart.r == std::numeric_limits<int16_t>::max())
			ptColorStart = particle.GetColor();
		colorStart = ptColorStart;
		componentFlags |= CParticleModifierComponentRandomColor::ComponentFlags::RGBA;
	}
	else {
		colorStart = m_colorStart.GetValue(particle);
		componentFlags |= m_colorStart.GetComponentFlags();
	}
	componentFlags &= m_colorEnd.GetComponentFlags();
	auto newColor = colorStart.Lerp(m_colorEnd.GetValue(particle), tFade);

	auto color = colorStart;
	if((componentFlags & CParticleModifierComponentRandomColor::ComponentFlags::Red) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.r = newColor.r;
	if((componentFlags & CParticleModifierComponentRandomColor::ComponentFlags::Green) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.g = newColor.g;
	if((componentFlags & CParticleModifierComponentRandomColor::ComponentFlags::Blue) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.b = newColor.b;
	if((componentFlags & CParticleModifierComponentRandomColor::ComponentFlags::Alpha) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.a = newColor.a;
	particle.SetColor(color);
}
