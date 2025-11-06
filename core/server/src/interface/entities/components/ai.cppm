// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.server:entities.components.ai;

import :ai;
import :entities.components.entity;
import :entities.components.player;

export {
	constexpr float AI_NEXT_ENEMY_CHECK_IDLE = 0.25f;
	constexpr float AI_NEXT_ENEMY_CHECK_ALERT = 0.1f;

	constexpr float AI_LISTEN_VISIBILITY_THRESHOLD = 4.f;
	constexpr float AI_LISTEN_DISTANCE_THRESHOLD = 100.f;

	class DLLSERVER NPCRelationship {
	  public:
		NPCRelationship(const std::shared_ptr<void> &userData, int pPriority = 0) : data(userData), priority(pPriority) {}
		std::shared_ptr<void> data;
		int priority;
	};

	namespace pragma {
		struct DLLSERVER CEMemoryData : public ComponentEvent {
			CEMemoryData(const ai::Memory::Fragment *memoryFragment);
			virtual void PushArguments(lua::State *l) override;
			const ai::Memory::Fragment *memoryFragment;
		};
		struct DLLSERVER CEOnNPCStateChanged : public ComponentEvent {
			CEOnNPCStateChanged(NPCSTATE oldState, NPCSTATE newState);
			virtual void PushArguments(lua::State *l) override;
			NPCSTATE oldState;
			NPCSTATE newState;
		};
		struct DLLSERVER CEOnTargetAcquired : public ComponentEvent {
			CEOnTargetAcquired(pragma::ecs::BaseEntity *entity, float distance, bool isFirstNewTarget);
			virtual void PushArguments(lua::State *l) override;
			pragma::ecs::BaseEntity *entity;
			float distance;
			bool isFirstNewTarget;
		};
		struct DLLSERVER CEOnControllerActionInput : public ComponentEvent {
			CEOnControllerActionInput(pragma::Action action, bool pressed);
			virtual void PushArguments(lua::State *l) override;
			pragma::Action action;
			bool pressed;
		};
		struct DLLSERVER CEOnSuspiciousSoundHeared : public ComponentEvent {
			CEOnSuspiciousSoundHeared(const std::shared_ptr<ALSound> &sound);
			virtual void PushArguments(lua::State *l) override;
			std::shared_ptr<ALSound> sound;
		};
		struct DLLSERVER CEOnStartControl : public ComponentEvent {
			CEOnStartControl(pragma::SPlayerComponent &player);
			virtual void PushArguments(lua::State *l) override;
			pragma::SPlayerComponent &player;
		};
		struct DLLSERVER CEOnPathNodeChanged : public ComponentEvent {
			CEOnPathNodeChanged(uint32_t nodeIndex);
			virtual void PushArguments(lua::State *l) override;
			uint32_t nodeIndex;
		};
		struct DLLSERVER CEOnScheduleStateChanged : public ComponentEvent {
			CEOnScheduleStateChanged(const std::shared_ptr<ai::Schedule> &schedule, ai::BehaviorNode::Result result);
			virtual void PushArguments(lua::State *l) override;
			std::shared_ptr<ai::Schedule> schedule;
			ai::BehaviorNode::Result result;
		};
		namespace sAIComponent {
			STATIC_DLL_COMPAT ComponentEventId EVENT_SELECT_SCHEDULE;
			STATIC_DLL_COMPAT ComponentEventId EVENT_SELECT_CONTROLLER_SCHEDULE;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_SCHEDULE_COMPLETE;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_PRIMARY_TARGET_CHANGED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_PATH_CHANGED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_NPC_STATE_CHANGED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_TARGET_VISIBILITY_LOST;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_TARGET_VISIBILITY_REACQUIRED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_MEMORY_GAINED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_MEMORY_LOST;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_TARGET_ACQUIRED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_SUSPICIOUS_SOUND_HEARED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_CONTROLLER_ACTION_INPUT;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_START_CONTROL;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_END_CONTROL;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_PATH_NODE_CHANGED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_LOOK_TARGET_CHANGED;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_SCHEDULE_STARTED;
		}
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

			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

			SAIComponent(pragma::ecs::BaseEntity &ent);
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
			void SetRelationship(pragma::ecs::BaseEntity *ent, DISPOSITION disp, bool revert = true, int priority = 0);
			void SetRelationship(EntityHandle &hEnt, DISPOSITION disp, bool revert = true, int priority = 0);
			void SetRelationship(std::string className, DISPOSITION disp, int priority = 0);
			void SetRelationship(Faction &faction, DISPOSITION disp, int priority = 0);
			void ClearRelationship(pragma::ecs::BaseEntity *ent);
			void ClearRelationship(EntityHandle &hEnt);
			void ClearRelationship(std::string className);
			void ClearRelationship(Faction &faction);
			DISPOSITION GetDisposition(pragma::ecs::BaseEntity *ent, int *priority = nullptr);
			DISPOSITION GetDisposition(EntityHandle &hEnt, int *priority = nullptr);
			DISPOSITION GetDisposition(std::string className, int *priority = nullptr);
			DISPOSITION GetDisposition(Faction &faction, int *priority = nullptr);
			std::shared_ptr<ai::Schedule> GetCurrentSchedule();
			void StartSchedule(std::shared_ptr<ai::Schedule> &sched);
			virtual void OnTick(double tDelta) override;
			ai::Memory &GetMemory();
			ai::Memory::Fragment *GetMemory(pragma::ecs::BaseEntity *ent);
			ai::Memory::Fragment *Memorize(pragma::ecs::BaseEntity *ent, ai::Memory::MemoryType memType);
			ai::Memory::Fragment *Memorize(pragma::ecs::BaseEntity *ent, ai::Memory::MemoryType memType, const Vector3 &pos, const Vector3 &vel);
			void Forget(pragma::ecs::BaseEntity *ent);
			void ClearMemory();
			bool IsInMemory(pragma::ecs::BaseEntity *ent);
			// Returns the number of occupied memory fragments
			uint32_t GetMemoryFragmentCount() const;
			bool IsInViewCone(pragma::ecs::BaseEntity *ent, float *dist = nullptr);
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
			pragma::Action GetControllerActionInput() const;
			bool IsControllable() const;
			void SetControllable(bool b);
			void StartControl(pragma::SPlayerComponent &pl);
			void EndControl();
			bool IsControlled() const;
			pragma::SPlayerComponent *GetController() const;
			bool IsEnemy(pragma::ecs::BaseEntity *ent) const;
			bool TriggerScheduleInterrupt(uint32_t interruptFlags);

			virtual void SendSnapshotData(NetPacket &packet, pragma::BasePlayerComponent &pl) override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitSnapshotData() const override { return true; }
			virtual bool ShouldTransmitNetData() const override { return true; }

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

				void SetActivity(pragma::Activity activity) const;
				void SetAnimation(int32_t animation) const;

				void SetFaceTarget(bool primaryTarget);
				void SetFaceTarget(const Vector3 &position);
				void SetFaceTarget(pragma::ecs::BaseEntity &target);

				// For internal use only
				int32_t GetAnimation() const;
				pragma::Activity GetActivity() const;
				const Vector3 *GetFacePosition() const;
				pragma::ecs::BaseEntity *GetEntityFaceTarget() const;
			  private:
				union {
					int32_t animation;
					pragma::Activity activity = pragma::Activity::Invalid;
				} mutable m_animation;
				pragma::FPlayAnim m_flags = pragma::FPlayAnim::Default;
				mutable AIAnimFlags m_aiAnimFlags = AIAnimFlags::Default;

				mutable std::shared_ptr<void> m_faceTarget = nullptr;
			};

			bool PlayActivity(pragma::Activity act, const AIAnimationInfo &info);
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
				pragma::Action actions = pragma::Action::None;
			};

			struct TargetInfo {
				TargetInfo(pragma::ecs::BaseEntity *_ent, float _dist) : ent(_ent), dist(_dist) {}
				pragma::ecs::BaseEntity *ent = nullptr;
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
			virtual void InitializeLuaObject(lua::State *l) override;
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
			virtual void OnTargetAcquired(pragma::ecs::BaseEntity *ent, float dist, bool bFirst);
			virtual bool OnSuspiciousSoundHeared(std::shared_ptr<ALSound> &snd);
			virtual void OnControllerActionInput(pragma::Action action, bool b);
			virtual void OnStartControl(pragma::SPlayerComponent &pl);
			virtual void OnEndControl();
			virtual void OnPathNodeChanged(uint32_t nodeIdx) override;
			virtual void OnLookTargetChanged() override;
			virtual bool IsObstruction(const pragma::ecs::BaseEntity &ent) const override;
			virtual void UpdateMovementProperties(MovementComponent &movementC) override;
			void OnTakenDamage(DamageInfo &info, unsigned short oldHealth, unsigned short newHealth);
			void OnTakeDamage(DamageInfo &info);
			void MaintainAnimationMovement(const Vector3 &disp);
			bool OnInput(std::string input, pragma::ecs::BaseEntity *activator, pragma::ecs::BaseEntity *caller, const std::string &data);
			void OnKilled(DamageInfo *damageInfo = nullptr);
			bool HasCharacterNoTargetEnabled(const pragma::ecs::BaseEntity &ent) const;

			// Animation
			bool PlayAnimation(const AIAnimationInfo &info);
			bool m_bSkipHandling = false;
		};
		using namespace umath::scoped_enum::bitwise;
	};
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::SAIComponent::AIAnimationInfo::AIAnimFlags> : std::true_type {};
	}
};
