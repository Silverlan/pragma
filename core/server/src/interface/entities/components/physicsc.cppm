// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.physics;

import :entities.components.entity;

export namespace pragma {
	namespace sPhysicsComponent {
		using namespace basePhysicsComponent;
	}
	class DLLSERVER SPhysicsComponent final : public BasePhysicsComponent, public SBaseNetComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		SPhysicsComponent(pragma::ecs::BaseEntity &ent) : BasePhysicsComponent(ent) {}

		virtual void OnPhysicsInitialized() override;
		virtual void OnPhysicsDestroyed() override;
		virtual bool PostPhysicsSimulate() override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual void SetCollisionsEnabled(bool b) override;
		virtual void SetSimulationEnabled(bool b) override;
		using BasePhysicsComponent::SetCollisionsEnabled;
		virtual void SetKinematic(bool b) override;

		virtual void SetCollisionFilter(pragma::physics::CollisionMask filterGroup, pragma::physics::CollisionMask filterMask) override;

		virtual void SetMoveType(pragma::physics::MOVETYPE movetype) override;
		void SetCollisionType(pragma::physics::COLLISIONTYPE collisiontype) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
