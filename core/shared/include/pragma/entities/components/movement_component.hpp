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
		static ComponentEventId EVENT_CALC_MOVEMENT_SPEED;
		static ComponentEventId EVENT_CALC_AIR_MOVEMENT_MODIFIER;
		static ComponentEventId EVENT_CALC_MOVEMENT_ACCELERATION;
		static ComponentEventId EVENT_CALC_MOVEMENT_DIRECTION;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		MovementComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		const Vector3 &GetMoveVelocity() const;
		void SetMoveVelocity(const Vector3 &vel);

		// Velocity minus ground velocity (Velocity caused by ground object)
		Vector3 GetLocalVelocity() const;

		// Calculates the forward and sideways movement speed
		virtual Vector2 CalcMovementSpeed() const;
		virtual float CalcAirMovementModifier() const;
		virtual float CalcMovementAcceleration(float &optOutRampUpTime) const;
		virtual Vector3 CalcMovementDirection(const Vector3 &forward, const Vector3 &right) const;

		virtual bool UpdateMovement();
		float GetMovementBlendScale() const;
		bool CanMove() const;
	  protected:
		virtual void InitializeLuaObject(lua_State *lua) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		Vector3 m_moveVelocity = {};
		float m_timeSinceMovementStart = 0.f;
		OrientationComponent *m_orientationComponent = nullptr;
		BaseCharacterComponent *m_charComponent = nullptr;
	};
	struct DLLNETWORK CECalcMovementSpeed : public ComponentEvent {
		CECalcMovementSpeed();
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		Vector2 speed = {};
	};
	struct DLLNETWORK CECalcAirMovementModifier : public ComponentEvent {
		CECalcAirMovementModifier();
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		float airMovementModifier = 0.f;
	};
	struct DLLNETWORK CECalcMovementAcceleration : public ComponentEvent {
		CECalcMovementAcceleration();
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		float acceleration = 0.f;
		float rampUpTime = 0.f;
	};
	struct DLLNETWORK CECalcMovementDirection : public ComponentEvent {
		CECalcMovementDirection(const Vector3 &forward, const Vector3 &right);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		const Vector3 &forward;
		const Vector3 &right;
		Vector3 direction = {};
	};
};

#endif
