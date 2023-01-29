/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASETRIGGERGRAVITY_HPP__
#define __BASETRIGGERGRAVITY_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <mathutil/uvec.h>
#include <memory>

struct DLLNETWORK GravitySettings {
	float scale = 1.f;
	std::shared_ptr<Vector3> dir = nullptr;
	std::shared_ptr<Vector3> dirMove = nullptr;
	std::shared_ptr<float> force = nullptr;
};

class BaseEntity;
namespace pragma {
	namespace Entity {
		namespace TriggerGravity {
			enum class SpawnFlags : uint32_t { AffectDirection = 2048, ResetOnEndTouch = AffectDirection << 1, ChangeOrientation = ResetOnEndTouch << 1 };

			enum class NetFlags : uint8_t { None = 0, StartTouch = 1, UseForce = StartTouch << 1 };

			DLLNETWORK void apply_gravity(BaseEntity *ent, uint32_t flags, const Vector3 &gravityDir, const Vector3 &dirUp, bool bUseForce, float gravityForce, std::shared_ptr<Vector3> *upDir = nullptr);
		};
	};
	class DLLNETWORK BaseEntityTriggerGravityComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
	  protected:
		virtual void OnResetGravity(BaseEntity *ent, GravitySettings &settings);
		virtual void OnStartTouch(BaseEntity *ent);
		void OnEndTouch(BaseEntity *ent);

		Vector3 m_kvGravityDir = {};
		float m_kvGravityForce = 0.f;
		bool m_kvUseForce = false;
		std::unordered_map<std::shared_ptr<EntityHandle>, GravitySettings> m_gravityReset;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::Entity::TriggerGravity::SpawnFlags);
REGISTER_BASIC_BITWISE_OPERATORS(pragma::Entity::TriggerGravity::NetFlags);

#endif
