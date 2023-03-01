/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_CHARACTER_COMPONENT_HPP__
#define __BASE_CHARACTER_COMPONENT_HPP__

#include "pragma/entities/components/base_actor_component.hpp"
#include "pragma/model/animation/activities.h"
#include "pragma/ai/navsystem.h"
#include "pragma/entities/baseentity_handle.h"
#include <pragma/math/orientation.h>
#include <mathutil/uvec.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <sharedutils/property/util_property_vector.h>

struct AnimationEvent;
namespace pragma {
	struct DLLNETWORK CEOnDeployWeapon : public ComponentEvent {
		CEOnDeployWeapon(BaseEntity &entWeapon);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity &weapon;
	};
	struct DLLNETWORK CEOnSetActiveWeapon : public ComponentEvent {
		CEOnSetActiveWeapon(BaseEntity *entWeapon);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity *weapon;
	};
	struct DLLNETWORK CEOnSetCharacterOrientation : public ComponentEvent {
		CEOnSetCharacterOrientation(const Vector3 &up);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &up;
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
	struct DLLNETWORK CEIsMoving : public ComponentEvent {
		CEIsMoving();
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		bool moving = false;
	};
	struct DLLNETWORK CEViewRotation : public ComponentEvent {
		CEViewRotation(const Quat &rotation);
		virtual void PushArguments(lua_State *l) override;
		const Quat &rotation;
	};
	class DLLNETWORK BaseCharacterComponent : public BaseActorComponent {
	  public:
		static ComponentEventId EVENT_ON_FOOT_STEP;
		static ComponentEventId EVENT_ON_CHARACTER_ORIENTATION_CHANGED;
		static ComponentEventId EVENT_ON_DEPLOY_WEAPON;
		static ComponentEventId EVENT_ON_SET_ACTIVE_WEAPON;
		static ComponentEventId EVENT_PLAY_FOOTSTEP_SOUND;
		static ComponentEventId EVENT_CALC_MOVEMENT_SPEED;
		static ComponentEventId EVENT_CALC_AIR_MOVEMENT_MODIFIER;
		static ComponentEventId EVENT_CALC_MOVEMENT_ACCELERATION;
		static ComponentEventId EVENT_CALC_MOVEMENT_DIRECTION;
		static ComponentEventId EVENT_IS_MOVING;
		static ComponentEventId EVENT_HANDLE_VIEW_ROTATION;
		static ComponentEventId EVENT_ON_JUMP;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		//
		enum class FootType : uint8_t { Left = 0, Right };

		BaseCharacterComponent(BaseEntity &ent);
		virtual ~BaseCharacterComponent() = default;
		virtual void FootStep(FootType foot);
		virtual Vector3 GetEyePosition() const;
		virtual Vector3 GetShootPosition() const;
		const Quat &GetViewOrientation() const;
		virtual void SetViewOrientation(const Quat &orientation);
		// Sets roll to 0 and normalizes pitch to range [-90,90]
		void NormalizeViewOrientation(Quat &inOutRot);
		const Quat &NormalizeViewOrientation();
		virtual void Initialize() override;
		TraceData GetAimTraceData(std::optional<float> maxDist = {}) const;
		EulerAngles GetViewAngles() const;
		void SetViewAngles(const EulerAngles &ang);
		Vector3 GetViewForward() const;
		Vector3 GetViewRight() const;
		Vector3 GetViewUp() const;
		void GetViewOrientation(Vector3 *forward, Vector3 *right, Vector3 *up = nullptr) const;
		void SetNeckControllers(const std::string &yawController, const std::string &pitchController);
		int32_t GetNeckYawBlendController() const;
		int32_t GetNeckPitchBlendController() const;
		const Vector3 &GetMoveVelocity() const;
		void SetMoveVelocity(const Vector3 &vel);
		// Velocity minus ground velocity (Velocity caused by ground object)
		Vector3 GetLocalVelocity() const;

		EulerAngles GetLocalOrientationAngles() const;
		Quat GetLocalOrientationRotation() const;
		EulerAngles GetLocalOrientationViewAngles() const;
		Quat GetLocalOrientationViewRotation() const;
		Quat WorldToLocalOrientation(const Quat &rot);
		EulerAngles WorldToLocalOrientation(const EulerAngles &ang);
		Quat LocalOrientationToWorld(const Quat &rot);
		EulerAngles LocalOrientationToWorld(const EulerAngles &ang);

		virtual void SetCharacterOrientation(const Vector3 &up);

		virtual void OnTick(double tDelta) override;

		const util::PVector3Property &GetUpDirectionProperty() const;
		const Vector3 &GetUpDirection() const;
		virtual void SetUpDirection(const Vector3 &direction);
		// Returns the forward,right and up vectors, depending on the player's up direction
		void GetOrientationAxes(Vector3 **forward, Vector3 **right, Vector3 **up);
		// Returns the rotation between the world's up vector and the player's up direction
		const Quat &GetOrientationAxesRotation() const;

		float GetTurnSpeed() const;
		void SetTurnSpeed(float speed);
		float GetSlopeLimit() const;
		virtual void SetSlopeLimit(float limit);
		const util::PFloatProperty &GetSlopeLimitProperty() const;
		float GetStepOffset() const;
		virtual void SetStepOffset(float offset);
		const util::PFloatProperty &GetStepOffsetProperty() const;
		virtual bool UpdateMovement();
		virtual bool IsCharacter() const;
		virtual bool IsMoving() const;

		// Ammo
		UInt16 GetAmmoCount(const std::string &ammoType) const;
		UInt16 GetAmmoCount(UInt32 ammoType) const;
		void SetAmmoCount(const std::string &ammoType, UInt16 count);
		virtual void SetAmmoCount(UInt32 ammoType, UInt16 count);
		void AddAmmo(const std::string &ammoType, int16_t count);
		void AddAmmo(UInt32 ammoType, int16_t count);
		void RemoveAmmo(const std::string &ammoType, int16_t count);
		void RemoveAmmo(UInt32 ammoType, int16_t count);
		//

		// Weapons
		std::vector<EntityHandle> &GetWeapons();
		virtual void GiveWeapon(BaseEntity &ent);
		BaseEntity *GiveWeapon(std::string className);
		BaseEntity *GetActiveWeapon();
		BaseEntity *GetWeapon(std::string className);
		std::vector<BaseEntity *> GetWeapons(std::string className);
		void GetWeapons(std::string className, std::vector<BaseEntity *> &weapons);
		void RemoveWeapon(std::string className);
		virtual std::vector<EntityHandle>::iterator RemoveWeapon(BaseEntity &ent);
		virtual void RemoveWeapons();
		void DeployWeapon(const std::string &className);
		virtual void DeployWeapon(BaseEntity &ent);
		// Don't call this directly!
		virtual void SetActiveWeapon(BaseEntity *ent);
		virtual void HolsterWeapon();
		bool HasWeapon(std::string className);
		virtual void PrimaryAttack();
		virtual void SecondaryAttack();
		virtual void TertiaryAttack();
		virtual void Attack4();
		virtual void ReloadWeapon();
		bool CanMove() const;

		float GetMovementBlendScale() const;

		// Unsticks the character from ground (disabling friction and also making sure gravity is applied) for the specified duration.
		void DetachFromGround(float duration = 0.1f);

		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		bool Jump();
		bool Jump(const Vector3 &velocity);
		float GetJumpPower() const;
		const util::PFloatProperty &GetJumpPowerProperty() const;
		void SetJumpPower(float power);
		bool CanJump() const;
	  protected:
		pragma::NetEventId m_netEvSetActiveWeapon = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetAmmoCount = pragma::INVALID_NET_EVENT;

		util::PVector3Property m_upDirection = nullptr;
		Vector3 m_axForward = {0.f, 0.f, 1.f};
		Vector3 m_axRight = {-1.f, 0.f, 0.f};
		Quat m_axRot = uquat::identity();
		float m_turnSpeed = 300.f;
		float m_tDetachFromGround = 0.f;
		util::PFloatProperty m_jumpPower = nullptr;
		std::unique_ptr<float> m_turnYaw = nullptr;
		Quat m_angView = uquat::identity();
		util::PFloatProperty m_slopeLimit = nullptr;
		util::PFloatProperty m_stepOffset = nullptr;
		Vector3 m_moveVelocity = {};
		std::vector<EntityHandle> m_weapons;
		EntityHandle m_weaponActive;
		std::unordered_map<UInt32, UInt16> m_ammoCount;

		int32_t m_yawController = -1;
		int32_t m_pitchController = -1;
		std::string m_yawControllerName = "aim_yaw";
		std::string m_pitchControllerName = "aim_pitch";

		// PhysX
		virtual void InitializeController();
		virtual void InitializePhysObj(PhysObj *phys);

		bool HandleAnimationEvent(const AnimationEvent &ev);
		virtual void PlayFootStepSound(FootType foot, const SurfaceMaterial &surfMat, float scale);
		// Calculates the forward and sideways movement speed
		virtual Vector2 CalcMovementSpeed() const;
		virtual float CalcAirMovementModifier() const;
		virtual float CalcMovementAcceleration() const;
		virtual Vector3 CalcMovementDirection(const Vector3 &forward, const Vector3 &right) const;
		void UpdateNeckControllers();

		virtual void UpdateOrientation();
	};
	struct DLLNETWORK CEPlayFootstepSound : public ComponentEvent {
		CEPlayFootstepSound(BaseCharacterComponent::FootType footType, const SurfaceMaterial &surfaceMaterial, float scale);
		virtual void PushArguments(lua_State *l) override;
		BaseCharacterComponent::FootType footType;
		const SurfaceMaterial &surfaceMaterial;
		float scale;
	};
	struct DLLNETWORK CEOnFootStep : public ComponentEvent {
		CEOnFootStep(BaseCharacterComponent::FootType footType);
		virtual void PushArguments(lua_State *l) override;
		BaseCharacterComponent::FootType footType;
	};
	struct DLLNETWORK CEOnJump : public ComponentEvent {
		CEOnJump(const Vector3 &velocity);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &velocity;
	};
};

#endif
