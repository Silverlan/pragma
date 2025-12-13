// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_player;

export import :entities.components.base;
export import :console.enums;
import :console.output;
export import :game.damage_info;
export import :input.enums;
export import :model.animation.enums;
export import :physics.shape;

export {
	class BasePlayer;
	namespace pragma {
		class Engine;
		class ActionInputControllerComponent;
		class BaseObservableComponent;
		class DLLNETWORK BasePlayerComponent : public BaseEntityComponent {
		  public:
			friend Engine;
			virtual ~BasePlayerComponent() override;
			virtual Con::c_cout &print(Con::c_cout &);
			virtual std::ostream &print(std::ostream &);

			virtual void OnTakenDamage(game::DamageInfo &info, unsigned short oldHealth, unsigned short newHealth);
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
			virtual void PrintMessage(std::string message, console::MESSAGE type) = 0;
			void GetConVars(std::unordered_map<std::string, std::string> **convars);
			bool GetConVar(std::string cvar, std::string *val);
			std::string GetConVarString(std::string cvar) const;
			int GetConVarInt(std::string cvar) const;
			float GetConVarFloat(std::string cvar) const;
			bool GetConVarBool(std::string cvar) const;
			ecs::BaseEntity *FindUseEntity() const;
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

			ActionInputControllerComponent *GetActionInputController();
			const ActionInputControllerComponent *GetActionInputController() const { return const_cast<BasePlayerComponent *>(this)->GetActionInputController(); }

			BasePlayer *GetBasePlayer() const;
			virtual void OnEntitySpawn() override;
		  protected:
			enum class CrouchTransition : int32_t {
				None = -1,
				Crouching = 0,
				Uncrouching = 1,
			};
			BasePlayerComponent(ecs::BaseEntity &ent);
			void UpdateMovementProperties();
			virtual void OnPhysicsInitialized();
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
			virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
			void HandleActionInput(Action action, bool pressed);
			void OnRespawn();
			bool m_bFlashlightOn;
			EntityHandle m_entFlashlight = {};
			mutable EntityHandle m_hBasePlayer = {};
			BaseObservableComponent *m_observableComponent = nullptr;
			ActionInputControllerComponent *m_actionController = nullptr;

			NetEventId m_netEvApplyViewRotationOffset = INVALID_NET_EVENT;
			NetEventId m_netEvPrintMessage = INVALID_NET_EVENT;
			NetEventId m_netEvRespawn = INVALID_NET_EVENT;
			NetEventId m_netEvSetViewOrientation = INVALID_NET_EVENT;

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
			void OnKilled(game::DamageInfo *dmgInfo = nullptr);

			virtual void OnTick(double tDelta) override;
			std::shared_ptr<physics::IConvexShape> m_shapeStand = nullptr;
		  private:
			unsigned short m_portUDP;
			std::unordered_map<int, bool> m_keysPressed;
			bool m_bLocalPlayer;
			std::unordered_map<std::string, std::string> m_conVars;

			// Movement
			float m_speedWalk;
			float m_speedRun;
			float m_speedSprint;
			float m_speedCrouchWalk;
			std::optional<Vector3> m_viewPos {};
		};
	};
}
