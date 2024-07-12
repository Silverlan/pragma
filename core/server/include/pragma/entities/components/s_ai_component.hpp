/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_AI_COMPONENT_HPP__
#define __S_AI_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_memory.h"
#include "pragma/ai/s_factions.h"
#include "pragma/ai/s_disposition.h"
#include "pragma/ai/ai_behavior.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/model/animation/play_animation_flags.hpp>
#include <pragma/entities/components/base_ai_component.hpp>
#include <sharedutils/util_weak_handle.hpp>

#define AI_NEXT_ENEMY_CHECK_IDLE 0.25f
#define AI_NEXT_ENEMY_CHECK_ALERT 0.1f

#define AI_LISTEN_VISIBILITY_THRESHOLD 4.f
#define AI_LISTEN_DISTANCE_THRESHOLD 100.f

class DLLSERVER NPCRelationship {
  public:
	NPCRelationship(const std::shared_ptr<void> &userData, int pPriority = 0) : data(userData), priority(pPriority) {}
	std::shared_ptr<void> data;
	int priority;
};

enum class DISPOSITION : uint32_t;
enum class NPCSTATE : int;
class AISquad;
struct DebugBehaviorTreeNode;
namespace pragma {
	class SCharacterComponent;
	class BaseActorComponent;
	class SPlayerComponent;
	enum class FPlayAnim : uint32_t;
};

class SBaseEntity;
namespace pragma {
	namespace ai {
		class Schedule;
		class BehaviorNode;
	};
	struct DLLSERVER CEMemoryData : public ComponentEvent {
		CEMemoryData(const ai::Memory::Fragment *memoryFragment);
		virtual void PushArguments(lua_State *l) override;
		const ai::Memory::Fragment *memoryFragment;
	};
	struct DLLSERVER CEOnNPCStateChanged : public ComponentEvent {
		CEOnNPCStateChanged(NPCSTATE oldState, NPCSTATE newState);
		virtual void PushArguments(lua_State *l) override;
		NPCSTATE oldState;
		NPCSTATE newState;
	};
	struct DLLSERVER CEOnTargetAcquired : public ComponentEvent {
		CEOnTargetAcquired(BaseEntity *entity, float distance, bool isFirstNewTarget);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity *entity;
		float distance;
		bool isFirstNewTarget;
	};
	struct DLLSERVER CEOnControllerActionInput : public ComponentEvent {
		CEOnControllerActionInput(Action action, bool pressed);
		virtual void PushArguments(lua_State *l) override;
		Action action;
		bool pressed;
	};
	struct DLLSERVER CEOnSuspiciousSoundHeared : public ComponentEvent {
		CEOnSuspiciousSoundHeared(const std::shared_ptr<ALSound> &sound);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<ALSound> sound;
	};
	struct DLLSERVER CEOnStartControl : public ComponentEvent {
		CEOnStartControl(pragma::SPlayerComponent &player);
		virtual void PushArguments(lua_State *l) override;
		pragma::SPlayerComponent &player;
	};
	struct DLLSERVER CEOnPathNodeChanged : public ComponentEvent {
		CEOnPathNodeChanged(uint32_t nodeIndex);
		virtual void PushArguments(lua_State *l) override;
		uint32_t nodeIndex;
	};
	struct DLLSERVER CEOnScheduleStateChanged : public ComponentEvent {
		CEOnScheduleStateChanged(const std::shared_ptr<ai::Schedule> &schedule, ai::BehaviorNode::Result result);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<ai::Schedule> schedule;
		ai::BehaviorNode::Result result;
	};
	class DLLSERVER SAIComponent final : public BaseAIComponent, public SBaseSnapshotComponent {
	  public:
		void _debugSendNavInfo(pragma::SPlayerComponent &pl);
		void _debugSendScheduleInfo(pragma::SPlayerComponent &pl, std::shared_ptr<DebugBehaviorTreeNode> &dbgTree, std::shared_ptr<ai::Schedule> &aiSchedule, float &tLastSchedUpdate);
		static std::vector<ComponentHandle<pragma::SPlayerComponent>> s_plDebugAiNav;
	  private:
		static std::vector<SAIComponent *> s_npcs;
		static FactionManager s_factionManager;
	  public:
		static FactionManager &GetFactionManager();
	  public:
		static unsigned int GetNPCCount();
		static const std::vector<SAIComponent *> &GetAll();

		static ComponentEventId EVENT_SELECT_SCHEDULE;
		static ComponentEventId EVENT_SELECT_CONTROLLER_SCHEDULE;
		static ComponentEventId EVENT_ON_SCHEDULE_COMPLETE;
		static ComponentEventId EVENT_ON_PRIMARY_TARGET_CHANGED;
		static ComponentEventId EVENT_ON_PATH_CHANGED;
		static ComponentEventId EVENT_ON_NPC_STATE_CHANGED;
		static ComponentEventId EVENT_ON_TARGET_VISIBILITY_LOST;
		static ComponentEventId EVENT_ON_TARGET_VISIBILITY_REACQUIRED;
		static ComponentEventId EVENT_ON_MEMORY_GAINED;
		static ComponentEventId EVENT_ON_MEMORY_LOST;
		static ComponentEventId EVENT_ON_TARGET_ACQUIRED;
		static ComponentEventId EVENT_ON_SUSPICIOUS_SOUND_HEARED;
		static ComponentEventId EVENT_ON_CONTROLLER_ACTION_INPUT;
		static ComponentEventId EVENT_ON_START_CONTROL;
		static ComponentEventId EVENT_ON_END_CONTROL;
		static ComponentEventId EVENT_ON_PATH_NODE_CHANGED;
		static ComponentEventId EVENT_ON_LOOK_TARGET_CHANGED;
		static ComponentEventId EVENT_ON_SCHEDULE_STARTED;

		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		SAIComponent(BaseEntity &ent);
		virtual ~SAIComponent() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		const ai::Memory::Fragment *GetPrimaryTarget() const;
		float GetMaxViewDistance() const;
		void SetMaxViewDistance(float dist);
		float GetMaxViewDotProduct() const;
		float GetMaxViewAngle() const;
		void SetMaxViewAngle(float ang);
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		void LockAnimation(bool b);
		bool IsAnimationLocked() const;
		void SetSquad(std::string squadName);
		std::string GetSquadName();
		NPCSTATE GetNPCState() const;
		void SetNPCState(NPCSTATE state);
		const std::shared_ptr<AISquad> &GetSquad() const;
		void SetRelationship(BaseEntity *ent, DISPOSITION disp, bool revert = true, int priority = 0);
		void SetRelationship(EntityHandle &hEnt, DISPOSITION disp, bool revert = true, int priority = 0);
		void SetRelationship(std::string className, DISPOSITION disp, int priority = 0);
		void SetRelationship(Faction &faction, DISPOSITION disp, int priority = 0);
		void ClearRelationship(BaseEntity *ent);
		void ClearRelationship(EntityHandle &hEnt);
		void ClearRelationship(std::string className);
		void ClearRelationship(Faction &faction);
		DISPOSITION GetDisposition(BaseEntity *ent, int *priority = nullptr);
		DISPOSITION GetDisposition(EntityHandle &hEnt, int *priority = nullptr);
		DISPOSITION GetDisposition(std::string className, int *priority = nullptr);
		DISPOSITION GetDisposition(Faction &faction, int *priority = nullptr);
		std::shared_ptr<ai::Schedule> GetCurrentSchedule();
		void StartSchedule(std::shared_ptr<ai::Schedule> &sched);
		virtual void OnTick(double tDelta) override;
		ai::Memory &GetMemory();
		ai::Memory::Fragment *GetMemory(BaseEntity *ent);
		ai::Memory::Fragment *Memorize(BaseEntity *ent, ai::Memory::MemoryType memType);
		ai::Memory::Fragment *Memorize(BaseEntity *ent, ai::Memory::MemoryType memType, const Vector3 &pos, const Vector3 &vel);
		void Forget(BaseEntity *ent);
		void ClearMemory();
		bool IsInMemory(BaseEntity *ent);
		// Returns the number of occupied memory fragments
		uint32_t GetMemoryFragmentCount() const;
		bool IsInViewCone(BaseEntity *ent, float *dist = nullptr);
		float GetMemoryDuration();
		void SetMemoryDuration(float dur);
		bool CanSee() const;
		void SetHearingStrength(float strength);
		bool CanHear() const;
		float GetHearingStrength() const;
		void CancelSchedule();
		virtual void OnScheduleComplete(const std::shared_ptr<ai::Schedule> &schedule, ai::BehaviorNode::Result result);
		virtual void OnScheduleStarted(const std::shared_ptr<ai::Schedule> &schedule, ai::BehaviorNode::Result result);
		virtual bool TurnStep(const Vector3 &target, float &turnAngle, const float *turnSpeed = nullptr) override;
		using BaseAIComponent::TurnStep;
		bool IsMoving() const;
		bool IsAIEnabled() const;
		void SetAIEnabled(bool b);
		void EnableAI();
		void DisableAI();
		Action GetControllerActionInput() const;
		bool IsControllable() const;
		void SetControllable(bool b);
		void StartControl(pragma::SPlayerComponent &pl);
		void EndControl();
		bool IsControlled() const;
		pragma::SPlayerComponent *GetController() const;
		bool IsEnemy(BaseEntity *ent) const;
		bool TriggerScheduleInterrupt(uint32_t interruptFlags);

		virtual void SendSnapshotData(NetPacket &packet, pragma::BasePlayerComponent &pl) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitSnapshotData() const override { return true; }
		virtual bool ShouldTransmitNetData() const { return true; }

		// Animation
		struct AIAnimationInfo {
			enum class AIAnimFlags : uint32_t {
				None = 0u,

				PlayAnimation = 1u,
				PlayActivity = PlayAnimation << 1u,

				FacePrimaryTarget = 1u,
				FacePosition = FacePrimaryTarget << 1u,
				FaceEntity = FacePosition << 1u,
				PlayAsSchedule = FaceEntity << 1u,

				Default = PlayAsSchedule
			};
			AIAnimationInfo() = default;

			void SetPlayFlags(pragma::FPlayAnim flags);
			pragma::FPlayAnim GetPlayFlags() const;
			AIAnimFlags GetAIAnimFlags() const;

			void SetPlayAsSchedule(bool playAsSchedule);
			bool ShouldPlayAsSchedule() const;

			void SetActivity(Activity activity) const;
			void SetAnimation(int32_t animation) const;

			void SetFaceTarget(bool primaryTarget);
			void SetFaceTarget(const Vector3 &position);
			void SetFaceTarget(BaseEntity &target);

			// For internal use only
			int32_t GetAnimation() const;
			Activity GetActivity() const;
			const Vector3 *GetFacePosition() const;
			BaseEntity *GetEntityFaceTarget() const;
		  private:
			union {
				int32_t animation;
				Activity activity = Activity::Invalid;
			} mutable m_animation;
			pragma::FPlayAnim m_flags = pragma::FPlayAnim::Default;
			mutable AIAnimFlags m_aiAnimFlags = AIAnimFlags::Default;

			mutable std::shared_ptr<void> m_faceTarget = nullptr;
		};

		bool PlayActivity(Activity act, const AIAnimationInfo &info);
		bool PlayAnimation(int32_t anim, const AIAnimationInfo &info);
	  protected:
		friend ai::BehaviorNode;

		struct ControlInfo {
			ControlInfo();
			void Clear();
			util::WeakHandle<pragma::SPlayerComponent> hController = {};
			CallbackHandle hCbOnRemove = {};
			CallbackHandle hCbOnKilled = {};
			CallbackHandle hCbOnActionInput = {};
			Action actions = Action::None;
		};

		struct TargetInfo {
			TargetInfo(BaseEntity *_ent, float _dist) : ent(_ent), dist(_dist) {}
			BaseEntity *ent = nullptr;
			float dist = 0.f;
		};

		bool m_bAnimLocked = false;
		ai::Memory m_memory;
		const ai::Memory::Fragment *m_primaryTarget = nullptr;
		std::shared_ptr<AISquad> m_squad = nullptr;
		NPCSTATE m_npcState = {};
		float m_maxViewDist = 8192.f;
		float m_maxViewAngle = 0.f;
		float m_maxViewDot = 0.f;
		float m_memoryDuration = 60.f;
		float m_tNextEnemyCheck = 0.f;
		float m_tNextListenCheck = 0.f;
		float m_hearingStrength = 0.f;
		bool m_bAiEnabled = true;
		bool m_bControllable = true;
		ControlInfo m_controlInfo = {};
		DISPOSITION GetDefaultDisposition();
		std::shared_ptr<ai::Schedule> m_schedule = nullptr;
		Vector3 m_posMove = {0, 0, 0};
		std::array<std::vector<std::shared_ptr<NPCRelationship>>, umath::to_integral(DISPOSITION::COUNT)> m_classRelationships;
		std::array<std::vector<std::shared_ptr<NPCRelationship>>, umath::to_integral(DISPOSITION::COUNT)> m_entityRelationships;
		std::array<std::vector<std::shared_ptr<NPCRelationship>>, umath::to_integral(DISPOSITION::COUNT)> m_factionRelationships;
		void ClearRelationships();
		virtual void OnRemove() override;
		virtual void RunSchedule();
		void UpdateMemory();
		void SelectEnemies();
		void Listen(std::vector<TargetInfo> &targets);
		void SelectPrimaryTarget();
		void OnPrePhysicsSimulate();
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnPrimaryTargetChanged(const ai::Memory::Fragment *memFragment);
		virtual void OnPathChanged() override;
		virtual void SelectSchedule();
		virtual void SelectControllerSchedule();
		virtual void OnNPCStateChanged(NPCSTATE oldState, NPCSTATE newState);
		virtual void OnTargetVisibilityLost(const ai::Memory::Fragment &memFragment);
		virtual void OnTargetVisibilityReacquired(const ai::Memory::Fragment &memFragment);
		virtual void OnMemoryGained(const ai::Memory::Fragment &memFragment);
		virtual void OnMemoryLost(const ai::Memory::Fragment &memFragment);
		virtual void OnTargetAcquired(BaseEntity *ent, float dist, bool bFirst);
		virtual bool OnSuspiciousSoundHeared(std::shared_ptr<ALSound> &snd);
		virtual void OnControllerActionInput(Action action, bool b);
		virtual void OnStartControl(pragma::SPlayerComponent &pl);
		virtual void OnEndControl();
		virtual void OnPathNodeChanged(uint32_t nodeIdx) override;
		virtual void OnLookTargetChanged() override;
		virtual bool IsObstruction(const BaseEntity &ent) const override;
		virtual void UpdateMovementProperties(MovementComponent &movementC) override;
		void OnTakenDamage(DamageInfo &info, unsigned short oldHealth, unsigned short newHealth);
		void OnTakeDamage(DamageInfo &info);
		void MaintainAnimationMovement(const Vector3 &disp);
		bool OnInput(std::string input, BaseEntity *activator, BaseEntity *caller, const std::string &data);
		void OnKilled(DamageInfo *damageInfo = nullptr);
		bool HasCharacterNoTargetEnabled(const BaseEntity &ent) const;

		// Animation
		bool PlayAnimation(const AIAnimationInfo &info);
		bool m_bSkipHandling = false;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::SAIComponent::AIAnimationInfo::AIAnimFlags)

#endif
