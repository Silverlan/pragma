// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_radius_fade;

export import :entities.components.particle_system;
export import :particle_system.modifier;
import :particle_system.modifier_gradual_fade;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorRadiusFadeBase : public CParticleOperator, public CParticleModifierComponentGradualFade {
	  public:
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double, float strength) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  protected:
		CParticleOperatorRadiusFadeBase(const std::string &identifier);
		virtual void ApplyRadius(CParticle &particle, float radius) const = 0;
	  private:
		CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fRadiusStart;
		CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>, float> m_fRadiusEnd;
		std::unique_ptr<std::vector<float>> m_particleStartRadiuses = nullptr;
		std::string m_identifier;
	};

	////////////////////////////

	class DLLCLIENT CParticleOperatorRadiusFade : public CParticleOperatorRadiusFadeBase {
	  public:
		CParticleOperatorRadiusFade();
	  protected:
		virtual void ApplyRadius(CParticle &particle, float radius) const override;
	};

	////////////////////////////

	class DLLCLIENT CParticleOperatorLengthFade : public CParticleOperatorRadiusFadeBase {
	  public:
		CParticleOperatorLengthFade();
	  protected:
		virtual void ApplyRadius(CParticle &particle, float radius) const override;
	};
};

pragma::pts::CParticleOperatorRadiusFadeBase::CParticleOperatorRadiusFadeBase(const std::string &identifier) : CParticleOperator {}, m_identifier {identifier} {}
void pragma::pts::CParticleOperatorRadiusFadeBase::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	CParticleModifierComponentGradualFade::Initialize(values);
	m_fRadiusStart.Initialize(m_identifier + "_start", values);
	m_fRadiusEnd.Initialize(m_identifier + "_end", values);

	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		string::to_lower(key);
		if(key == m_identifier) // Alternative to "radius_end"
			m_fRadiusEnd.SetRange(util::to_float(it->second));
	}

	// If no start radius has been specified, the previous known radius of the particle has to be used as start radius.
	// Since that radius cannot be known beforehand, we need to store it.
	if(m_fRadiusStart.IsSet() == false)
		m_particleStartRadiuses = std::make_unique<std::vector<float>>(static_cast<ecs::CParticleSystemComponent &>(pSystem).GetMaxParticleCount(), std::numeric_limits<float>::max());
}
void pragma::pts::CParticleOperatorRadiusFadeBase::OnParticleCreated(CParticle &particle)
{
	if(m_particleStartRadiuses == nullptr)
		return;
	m_particleStartRadiuses->at(particle.GetIndex()) = std::numeric_limits<float>::max();
}
void pragma::pts::CParticleOperatorRadiusFadeBase::Simulate(CParticle &particle, double, float strength)
{
	auto tFade = 0.f;
	if(GetEasedFadeFraction(particle, tFade) == false)
		return;
	auto radiusStart = 0.f;
	if(m_particleStartRadiuses != nullptr) {
		// Use last known particle radius
		auto &ptRadiusStart = m_particleStartRadiuses->at(particle.GetIndex());
		if(ptRadiusStart == std::numeric_limits<float>::max())
			ptRadiusStart = particle.GetRadius();
		radiusStart = ptRadiusStart;
	}
	else
		radiusStart = m_fRadiusStart.GetValue(particle);
	auto radiusEnd = m_fRadiusEnd.GetValue(particle);
	auto radius = radiusStart + (radiusEnd - radiusStart) * tFade;
	ApplyRadius(particle, radius);
}

////////////////////////////

pragma::pts::CParticleOperatorRadiusFade::CParticleOperatorRadiusFade() : CParticleOperatorRadiusFadeBase("radius") {}
void pragma::pts::CParticleOperatorRadiusFade::ApplyRadius(CParticle &particle, float radius) const { particle.SetRadius(radius); }

////////////////////////////

pragma::pts::CParticleOperatorLengthFade::CParticleOperatorLengthFade() : CParticleOperatorRadiusFadeBase("length") {}
void pragma::pts::CParticleOperatorLengthFade::ApplyRadius(CParticle &particle, float radius) const { particle.SetLength(radius); }
