// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.props.base;

export import :entities.components.base;
export import :physics.enums;

export namespace pragma {
	class DLLNETWORK BasePropComponent : public BaseEntityComponent {
	  public:
		enum class SpawnFlags : uint32_t { DisableCollisions = 2'048, Static = DisableCollisions << 1 };
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		physics::PhysicsType UpdatePhysicsType(ecs::BaseEntity *ent);
		void InitializePhysics(physics::PhysicsType physType);
		void Setup(physics::PhysicsType physType, physics::MoveType mvType);
		virtual void OnEntitySpawn() override;
	  protected:
		void InitializePhysics();
		Vector3 m_kvScale {1.f, 1.f, 1.f};
		float m_kvMass = std::numeric_limits<float>::quiet_NaN();
		BasePropComponent(ecs::BaseEntity &ent);
		bool SetKeyValue(std::string key, std::string val);

		physics::PhysicsType m_physicsType = physics::PhysicsType::None;
		physics::MoveType m_moveType = physics::MoveType::None;
	};
};
