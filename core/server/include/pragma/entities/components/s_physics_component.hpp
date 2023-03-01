/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_PHYSICS_COMPONENT_HPP__
#define __S_PHYSICS_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_physics_component.hpp>

namespace pragma {
	class DLLSERVER SPhysicsComponent final : public BasePhysicsComponent, public SBaseNetComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		SPhysicsComponent(BaseEntity &ent) : BasePhysicsComponent(ent) {}

		virtual void OnPhysicsInitialized() override;
		virtual void OnPhysicsDestroyed() override;
		virtual bool PostPhysicsSimulate() override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual void SetCollisionsEnabled(bool b) override;
		virtual void SetSimulationEnabled(bool b) override;
		using BasePhysicsComponent::SetCollisionsEnabled;
		virtual void SetKinematic(bool b) override;

		virtual void SetCollisionFilter(CollisionMask filterGroup, CollisionMask filterMask) override;

		virtual void SetMoveType(MOVETYPE movetype) override;
		void SetCollisionType(COLLISIONTYPE collisiontype) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};

#endif
