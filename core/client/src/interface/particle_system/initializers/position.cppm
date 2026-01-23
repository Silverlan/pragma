// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.initializer_position;

export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleInitializerPositionRandomBox : public CParticleInitializer {
	  public:
		CParticleInitializerPositionRandomBox() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  private:
		Vector3 m_min = {};
		Vector3 m_max = {};
		Vector3 m_origin = {};
		bool m_bOnSides = false;
	};

	//////////////////////////////

	class DLLCLIENT CParticleInitializerPositionRandomSphere : public CParticleInitializer {
	  public:
		CParticleInitializerPositionRandomSphere() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  private:
		float m_distMin = 0.f;
		float m_distMax = 0.f;
		Vector3 distBias = {1.f, 1.f, 1.f};
		Vector3 m_origin = {};
	};

	//////////////////////////////

	class DLLCLIENT CParticleInitializerPositionRandomCircle : public CParticleInitializer {
	  public:
		CParticleInitializerPositionRandomCircle() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  private:
		Vector3 m_vAxis = Vector3(0.f, 1.f, 0.f);
		float m_fMinDist = 0.f;
		float m_fMaxDist = 0.f;
		Vector3 m_origin = {};
	};
};
