// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.physics;

import :entities.components.entity;

export namespace pragma {
	namespace sPhysicsComponent {
		using namespace basePhysicsComponent;
	}
	class DLLSERVER SPhysicsComponent final : public BasePhysicsComponent, public SBaseNetComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		SPhysicsComponent(ecs::BaseEntity &ent) : BasePhysicsComponent(ent) {}

		virtual void OnPhysicsInitialized() override;
		virtual void OnPhysicsDestroyed() override;
		virtual bool PostPhysicsSimulate() override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual void SetCollisionsEnabled(bool b) override;
		virtual void SetSimulationEnabled(bool b) override;
		using BasePhysicsComponent::SetCollisionsEnabled;
		virtual void SetKinematic(bool b) override;

		virtual void SetCollisionFilter(physics::CollisionMask filterGroup, physics::CollisionMask filterMask) override;

		virtual void SetMoveType(physics::MoveType movetype) override;
		void SetCollisionType(physics::CollisionType collisiontype) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
