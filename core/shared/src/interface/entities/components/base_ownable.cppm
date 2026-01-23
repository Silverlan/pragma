// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_ownable;

export import :entities.components.base;
export import :entities.property;

export namespace pragma {
	struct DLLNETWORK CEOnOwnerChanged : public ComponentEvent {
		CEOnOwnerChanged(ecs::BaseEntity *oldOwner, ecs::BaseEntity *newOwner);
		virtual void PushArguments(lua::State *l) override;
		ecs::BaseEntity *oldOwner;
		ecs::BaseEntity *newOwner;
	};
	namespace baseOwnableComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_OWNER_CHANGED)
	}
	class DLLNETWORK BaseOwnableComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		const PEntityProperty &GetOwnerProperty() const;
		void SetOwner(ecs::BaseEntity &owner);
		void ClearOwner();
		ecs::BaseEntity *GetOwner();
		const ecs::BaseEntity *GetOwner() const;
	  protected:
		BaseOwnableComponent(ecs::BaseEntity &ent);
		virtual void SetOwner(ecs::BaseEntity *owner);
		NetEventId m_netEvSetOwner = INVALID_NET_EVENT;
		PEntityProperty m_owner;
	};
};
