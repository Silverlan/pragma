// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <iostream>
#include <memory>

export module pragma.shared:entities.components.props.base;

export import :entities.components.base;
export import :physics.enums;

export namespace pragma {
	class DLLNETWORK BasePropComponent : public BaseEntityComponent {
	  public:
		enum class SpawnFlags : uint32_t { DisableCollisions = 2'048, Static = DisableCollisions << 1 };
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		pragma::physics::PHYSICSTYPE UpdatePhysicsType(pragma::ecs::BaseEntity *ent);
		void InitializePhysics(pragma::physics::PHYSICSTYPE physType);
		void Setup(pragma::physics::PHYSICSTYPE physType, pragma::physics::MOVETYPE mvType);
		virtual void OnEntitySpawn() override;
	  protected:
		void InitializePhysics();
		Vector3 m_kvScale {1.f, 1.f, 1.f};
		float m_kvMass = std::numeric_limits<float>::quiet_NaN();
		BasePropComponent(pragma::ecs::BaseEntity &ent);
		bool SetKeyValue(std::string key, std::string val);

		pragma::physics::PHYSICSTYPE m_physicsType = pragma::physics::PHYSICSTYPE::NONE;
		pragma::physics::MOVETYPE m_moveType = pragma::physics::MOVETYPE::NONE;
	};
};
