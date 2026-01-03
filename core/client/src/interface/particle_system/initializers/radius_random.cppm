// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.initializer_radius_random;

export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleInitializerRadiusRandomBase : public CParticleInitializer {
	  public:
		CParticleInitializerRadiusRandomBase(const std::string &identifier);
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
		float GetMinRadius() const;
		float GetMaxRadius() const;
	  protected:
		virtual void ApplyRadius(CParticle &particle, float radius) = 0;
	  private:
		float m_radiusMin = 0.f;
		float m_radiusMax = 0.f;
		std::string m_identifier;
	};

	////////////////////////////

	class DLLCLIENT CParticleInitializerRadiusRandom : public CParticleInitializerRadiusRandomBase {
	  public:
		CParticleInitializerRadiusRandom();
	  protected:
		virtual void ApplyRadius(CParticle &particle, float radius) override;
	};

	////////////////////////////

	class DLLCLIENT CParticleInitializerLengthRandom : public CParticleInitializerRadiusRandomBase {
	  public:
		CParticleInitializerLengthRandom();
	  protected:
		virtual void ApplyRadius(CParticle &particle, float radius) override;
	};
};
