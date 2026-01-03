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
		CEOnCharacterKilled(game::DamageInfo *damageInfo);
		virtual void PushArguments(lua::State *l) override;
		game::DamageInfo *damageInfo;
	};
	namespace baseActorComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_KILLED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_RESPAWN)
		REGISTER_COMPONENT_EVENT(EVENT_ON_DEATH)
	}
	class DLLNETWORK BaseActorComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		virtual void Kill(game::DamageInfo *dmgInfo = nullptr);
		virtual void Respawn();
		virtual void SetFrozen(bool b);
		bool IsFrozen() const;
		bool IsAlive() const;
		bool IsDead() const;
		void Ragdolize();
		bool FindHitgroup(const physics::ICollisionObject &phys, physics::HitGroup &hitgroup) const;
		PhysObjHandle GetHitboxPhysicsObject() const;

		const util::PBoolProperty &GetFrozenProperty() const;

		void SetMoveController(const std::string &moveController);
		void SetMoveController(const std::string &moveControllerX, const std::string &moveControllerY);
		int32_t GetMoveController() const;
		int32_t GetMoveControllerY() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		BaseActorComponent(ecs::BaseEntity &ent);
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
		NetEventId m_netEvSetFrozen = INVALID_NET_EVENT;
		std::vector<HitboxData> m_hitboxData;
		std::unique_ptr<physics::PhysObj> m_physHitboxes;
		virtual void OnPhysicsInitialized();
		virtual void OnPhysicsDestroyed();
		virtual void PhysicsUpdate(double tDelta);
		virtual void OnDeath(game::DamageInfo *dmgInfo);
		void UpdateHitboxPhysics();
		void UpdateMoveController();
		void InitializeMoveController();
	};
};
