// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_physics;

export import :particle_system.modifier;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorPhysics : public CParticleOperator {
	  protected:
		float m_mass = 0.f;
		float m_linearDamping = 0.f;
		Vector3 m_linearFactor = {1.f, 1.f, 1.f};
		float m_angularDamping = 0.f;
		Vector3 m_angularFactor = {1.f, 1.f, 1.f};
		Vector3 m_posOffset = {};
		Quat m_rotOffset = uquat::identity();
		std::vector<util::TSharedHandle<physics::IRigidBody>> m_physicsObjects;
		virtual std::shared_ptr<physics::IShape> CreateShape() = 0;
	  public:
		CParticleOperatorPhysics() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
		virtual void OnParticleSystemStarted() override;
		virtual void OnParticleDestroyed(CParticle &particle) override;
		virtual void OnParticleSystemStopped() override;
		virtual void PreSimulate(CParticle &particle, double) override;
		virtual void PostSimulate(CParticle &particle, double) override;
	};

	class DLLCLIENT CParticleOperatorPhysicsSphere : public CParticleOperatorPhysics {
	  protected:
		float m_radius = 0.f;
		virtual std::shared_ptr<physics::IShape> CreateShape() override;
	  public:
		CParticleOperatorPhysicsSphere() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	};

	class DLLCLIENT CParticleOperatorPhysicsBox : public CParticleOperatorPhysics {
	  protected:
		float m_extent = 0.f;
		virtual std::shared_ptr<physics::IShape> CreateShape() override;
	  public:
		CParticleOperatorPhysicsBox() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	};

	class DLLCLIENT CParticleOperatorPhysicsCylinder : public CParticleOperatorPhysics {
	  protected:
		float m_radius = 0.f;
		float m_height = 0.f;
		virtual std::shared_ptr<physics::IShape> CreateShape() override;
	  public:
		CParticleOperatorPhysicsCylinder() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	};

	class DLLCLIENT CParticleOperatorPhysicsModel : public CParticleOperatorPhysics {
	  public:
		CParticleOperatorPhysicsModel() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(CParticle &particle) override;
	  protected:
		std::shared_ptr<asset::Model> m_model = nullptr;
		virtual std::shared_ptr<physics::IShape> CreateShape() override;
	};
};
