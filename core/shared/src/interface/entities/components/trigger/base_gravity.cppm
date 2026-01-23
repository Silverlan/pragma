// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.triggers.base_gravity;

export import :entities.components.base;

export {
	struct DLLNETWORK GravitySettings {
		float scale = 1.f;
		std::shared_ptr<Vector3> dir = nullptr;
		std::shared_ptr<Vector3> dirMove = nullptr;
		std::shared_ptr<float> force = nullptr;
	};
	namespace pragma {
		namespace Entity {
			namespace TriggerGravity {
				enum class SpawnFlags : uint32_t { AffectDirection = 2048, ResetOnEndTouch = AffectDirection << 1, ChangeOrientation = ResetOnEndTouch << 1 };

				enum class NetFlags : uint8_t { None = 0, StartTouch = 1, UseForce = StartTouch << 1 };

				DLLNETWORK void apply_gravity(ecs::BaseEntity *ent, uint32_t flags, const Vector3 &gravityDir, const Vector3 &dirUp, bool bUseForce, float gravityForce, std::shared_ptr<Vector3> *upDir = nullptr);
			};
		};
		class DLLNETWORK BaseEntityTriggerGravityComponent : public BaseEntityComponent {
		  public:
			using BaseEntityComponent::BaseEntityComponent;
			virtual void Initialize() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		  protected:
			virtual void OnResetGravity(ecs::BaseEntity *ent, GravitySettings &settings);
			virtual void OnStartTouch(ecs::BaseEntity *ent);
			void OnEndTouch(ecs::BaseEntity *ent);

			Vector3 m_kvGravityDir = {};
			float m_kvGravityForce = 0.f;
			bool m_kvUseForce = false;
			std::unordered_map<std::shared_ptr<EntityHandle>, GravitySettings> m_gravityReset;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::Entity::TriggerGravity::SpawnFlags)
	REGISTER_ENUM_FLAGS(pragma::Entity::TriggerGravity::NetFlags)
};
