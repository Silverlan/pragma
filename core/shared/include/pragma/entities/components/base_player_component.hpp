/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_PLAYER_COMPONENT_HPP__
#define __BASE_PLAYER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/input/inkeys.h"
#include "pragma/emessage.h"
#include "pragma/model/animation/activities.h"
#include <sharedutils/property/util_property.hpp>

class BasePlayer;
enum class Activity : uint16_t;
namespace pragma {
	namespace physics {
		class IConvexShape;
	};
	struct DLLNETWORK CEHandleActionInput : public ComponentEvent {
		CEHandleActionInput(Action action, bool pressed, float magnitude);
		virtual void PushArguments(lua_State *l) override;
		Action action;
		bool pressed;
		float magnitude;
	};

	class BaseObservableComponent;
	class DLLNETWORK BasePlayerComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_HANDLE_ACTION_INPUT;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		friend Engine;
		virtual ~BasePlayerComponent() override;
		virtual Con::c_cout &print(Con::c_cout &);
		virtual std::ostream &print(std::ostream &);

		virtual void OnTakenDamage(DamageInfo &info, unsigned short oldHealth, unsigned short newHealth);
		// Same as PlayActivity, but doesn't automatically transmit to clients if called serverside
		virtual bool PlaySharedActivity(Activity activity);

		virtual void SetViewRotation(const Quat &rot);

		void SetFlashlight(bool b);
		void ToggleFlashlight();
		bool IsFlashlightOn() const;
		// Movement
		float GetWalkSpeed() const;
		float GetRunSpeed() const;
		float GetSprintSpeed() const;
		float GetCrouchedWalkSpeed() const;
		virtual void SetWalkSpeed(float speed);
		virtual void SetRunSpeed(float speed);
		virtual void SetSprintSpeed(float speed);
		virtual void SetCrouchedWalkSpeed(float speed);
		float GetStandHeight() const;
		float GetCrouchHeight() const;
		// Changes the height immediately if the player is currently standing; Make sure to check for overlaps first
		virtual void SetStandHeight(float height);
		// Changes the height immediately if the player is currently crouching; Make sure to check for overlaps first
		virtual void SetCrouchHeight(float height);
		float GetStandEyeLevel() const;
		float GetCrouchEyeLevel() const;
		virtual void SetStandEyeLevel(float eyelevel);
		virtual void SetCrouchEyeLevel(float eyelevel);

		// Inputs
		void SetActionInput(Action action, bool b, bool bKeepMagnitude);
		void SetActionInput(Action action, bool b, float magnitude = 1.f);
		bool GetActionInput(Action action) const;
		bool GetRawActionInput(Action action) const;
		float GetActionInputAxisMagnitude(Action action) const;
		const std::unordered_map<Action, float> &GetActionInputAxisMagnitudes() const;
		Action GetActionInputs() const;
		Action GetRawActionInputs() const;
		void SetActionInputs(Action action, bool bKeepMagnitudes = false);
		void SetActionInputAxisMagnitude(Action action, float magnitude);
		void Crouch();
		void UnCrouch(bool bForce = false);
		virtual void OnCrouch();
		virtual void OnUnCrouch();
		virtual void OnFullyCrouched();
		virtual void OnFullyUnCrouched();
		bool IsCrouching() const;
		bool CanUnCrouch() const;

		BaseObservableComponent *GetObservableComponent() { return m_observableComponent; }
		const BaseObservableComponent *GetObservableComponent() const { return const_cast<BasePlayerComponent *>(this)->GetObservableComponent(); }

		void SetUDPPort(unsigned short port);
		unsigned short GetUDPPort() const;
		bool IsLocalPlayer() const;
		virtual void SetLocalPlayer(bool b);
		virtual bool IsKeyDown(int key);
		virtual std::string GetClientIP();
		virtual unsigned short GetClientPort();
		double ConnectionTime() const;
		double TimeConnected() const;
		virtual void PrintMessage(std::string message, MESSAGE type) = 0;
		void GetConVars(std::unordered_map<std::string, std::string> **convars);
		bool GetConVar(std::string cvar, std::string *val);
		std::string GetConVarString(std::string cvar) const;
		int GetConVarInt(std::string cvar) const;
		float GetConVarFloat(std::string cvar) const;
		bool GetConVarBool(std::string cvar) const;
		BaseEntity *FindUseEntity() const;
		void Use();
		Vector3 GetViewPos() const;
		void SetViewPos(const std::optional<Vector3> &pos);

		// Returns true if a movement activity is currently playing
		bool IsMoving() const;
		// Returns true if the player is holding down the walk-key
		bool IsWalking() const;
		// Returns true if the player is holding down the sprint-key
		bool IsSprinting() const;

		virtual void ApplyViewRotationOffset(const EulerAngles &ang, float dur = 0.5f) = 0;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		BasePlayer *GetBasePlayer() const;
		virtual void OnEntitySpawn() override;
	  protected:
		enum class CrouchTransition : int32_t {
			None = -1,
			Crouching = 0,
			Uncrouching = 1,
		};
		BasePlayerComponent(BaseEntity &ent);
		virtual void OnPhysicsInitialized();
		void OnRespawn();
		bool m_bFlashlightOn;
		EntityHandle m_entFlashlight = {};
		mutable EntityHandle m_hBasePlayer = {};
		BaseObservableComponent *m_observableComponent = nullptr;

		pragma::NetEventId m_netEvApplyViewRotationOffset = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvPrintMessage = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvRespawn = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetViewOrientation = pragma::INVALID_NET_EVENT;

		double m_timeConnected;
		bool m_bForceAnimationUpdate = false;
		float m_standHeight;
		float m_crouchHeight;
		float m_standEyeLevel;
		float m_crouchEyeLevel;
		float m_tCrouch;
		CrouchTransition m_crouchTransition = CrouchTransition::None;
		bool m_bCrouching;
		Activity m_movementActivity = Activity::Invalid; // Current activity, if we're moving

		virtual void Initialize() override;

		Vector2 CalcMovementSpeed() const;
		float CalcAirMovementModifier() const;
		float CalcMovementAcceleration(float &optOutRampUpTime) const;
		virtual void OnActionInputChanged(Action action, bool b);
		void OnKilled(DamageInfo *dmgInfo = nullptr);

		virtual void OnTick(double tDelta) override;
		std::shared_ptr<pragma::physics::IConvexShape> m_shapeStand = nullptr;
	  private:
		void UpdateMovementProperties();
		unsigned short m_portUDP;
		std::unordered_map<int, bool> m_keysPressed;
		bool m_bLocalPlayer;
		std::unordered_map<std::string, std::string> m_conVars;
		// Inputs
		Action m_actionInputs = Action::None;
		Action m_rawInputs = Action::None;
		std::unordered_map<Action, float> m_inputAxes;
		// Movement
		float m_speedWalk;
		float m_speedRun;
		float m_speedSprint;
		float m_speedCrouchWalk;
		std::optional<Vector3> m_viewPos {};
	};
};

#endif
