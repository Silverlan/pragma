/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __MOVEMENT_COMPONENT_HPP__
#define __MOVEMENT_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class OrientationComponent;
	class BaseCharacterComponent;
	class DLLNETWORK MovementComponent final : public BaseEntityComponent {
	  public:
		enum class MoveDirection : uint8_t {
			Forward = 0,
			Right,
			Backward,
			Left,

			Count,
		};
		static ComponentEventId EVENT_ON_UPDATE_MOVEMENT;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		MovementComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		const Vector3 &GetMoveVelocity() const;
		void SetMoveVelocity(const Vector3 &vel);

		// Velocity minus ground velocity (Velocity caused by ground object)
		Vector3 GetLocalVelocity() const;

		void SetSpeed(const Vector2 &speed);
		const Vector2 &GetSpeed() const;

		void SetAirModifier(float modifier);
		float GetAirModifier() const;

		void SetAcceleration(float acc);
		float GetAcceleration() const;

		void SetAccelerationRampUpTime(float rampUpTime);
		float GetAccelerationRampUpTime() const;

		void SetDirection(const std::optional<Vector3> &dir);
		const std::optional<Vector3> &GetDirection() const;

		void SetDirectionMagnitude(MoveDirection direction, float magnitude);
		float GetDirectionMagnitude(MoveDirection direction) const;

		virtual bool UpdateMovement();
		float GetMovementBlendScale() const;
		bool CanMove() const;
	  protected:
		virtual void InitializeLuaObject(lua_State *lua) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		Vector2 m_movementSpeed {};
		float m_airMovementModifier = 0.f;
		float m_movementAcceleration = 0.f;
		float m_accelerationRampUpTime = 0.f;
		std::optional<Vector3> m_movementDirection {};
		std::array<float, umath::to_integral(MoveDirection::Count)> m_directionMagnitude {0.f, 0.f, 0.f, 0.f};

		Vector3 m_moveVelocity = {};
		float m_timeSinceMovementStart = 0.f;
		OrientationComponent *m_orientationComponent = nullptr;
		BaseCharacterComponent *m_charComponent = nullptr;
	};
};

#endif
