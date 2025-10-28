// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <memory>

#include <unordered_map>
#include "pragma/lua/core.hpp"

#include <ostream>

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

				DLLNETWORK void apply_gravity(pragma::ecs::BaseEntity *ent, uint32_t flags, const Vector3 &gravityDir, const Vector3 &dirUp, bool bUseForce, float gravityForce, std::shared_ptr<Vector3> *upDir = nullptr);
			};
		};
		class DLLNETWORK BaseEntityTriggerGravityComponent : public BaseEntityComponent {
		public:
			using BaseEntityComponent::BaseEntityComponent;
			virtual void Initialize() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		protected:
			virtual void OnResetGravity(pragma::ecs::BaseEntity *ent, GravitySettings &settings);
			virtual void OnStartTouch(pragma::ecs::BaseEntity *ent);
			void OnEndTouch(pragma::ecs::BaseEntity *ent);

			Vector3 m_kvGravityDir = {};
			float m_kvGravityForce = 0.f;
			bool m_kvUseForce = false;
			std::unordered_map<std::shared_ptr<EntityHandle>, GravitySettings> m_gravityReset;
		};
        using namespace umath::scoped_enum::bitwise;
	};
    namespace umath::scoped_enum::bitwise {
        template<>
        struct enable_bitwise_operators<pragma::Entity::TriggerGravity::SpawnFlags> : std::true_type {};
		
        template<>
        struct enable_bitwise_operators<pragma::Entity::TriggerGravity::NetFlags> : std::true_type {};
    }
};
