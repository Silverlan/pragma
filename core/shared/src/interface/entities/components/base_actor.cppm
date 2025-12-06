// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_actor;

export import :entities.components.base;
export import :game.damage_info;
import :physics.collision_object;
import :physics.object;

export namespace pragma {
	struct DLLNETWORK CEOnCharacterKilled : public ComponentEvent {
		CEOnCharacterKilled(DamageInfo *damageInfo);
		virtual void PushArguments(lua::State *l) override;
		DamageInfo *damageInfo;
	};
	namespace baseActorComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_KILLED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RESPAWN;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_DEATH;
	}
	class DLLNETWORK BaseActorComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		virtual void Kill(DamageInfo *dmgInfo = nullptr);
		virtual void Respawn();
		virtual void SetFrozen(bool b);
		bool IsFrozen() const;
		bool IsAlive() const;
		bool IsDead() const;
		void Ragdolize();
		bool FindHitgroup(const physics::ICollisionObject &phys, pragma::physics::HitGroup &hitgroup) const;
		PhysObjHandle GetHitboxPhysicsObject() const;

		const util::PBoolProperty &GetFrozenProperty() const;

		void SetMoveController(const std::string &moveController);
		void SetMoveController(const std::string &moveControllerX, const std::string &moveControllerY);
		int32_t GetMoveController() const;
		int32_t GetMoveControllerY() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		BaseActorComponent(pragma::ecs::BaseEntity &ent);
		bool m_bAlive;
		util::PBoolProperty m_bFrozen = nullptr;
		std::string m_moveControllerName = "move_yaw";
		std::optional<std::string> m_moveControllerNameY;
		int32_t m_moveController = -1;
		int32_t m_moveControllerY = -1;
		struct DLLNETWORK HitboxData {
			HitboxData(uint32_t boneId, const Vector3 &offset);
			HitboxData() = default;
			uint32_t boneId;
			Vector3 offset;
		};
		pragma::NetEventId m_netEvSetFrozen = pragma::INVALID_NET_EVENT;
		std::vector<HitboxData> m_hitboxData;
		std::unique_ptr<pragma::physics::PhysObj> m_physHitboxes;
		virtual void OnPhysicsInitialized();
		virtual void OnPhysicsDestroyed();
		virtual void PhysicsUpdate(double tDelta);
		virtual void OnDeath(DamageInfo *dmgInfo);
		void UpdateHitboxPhysics();
		void UpdateMoveController();
		void InitializeMoveController();
	};
};
