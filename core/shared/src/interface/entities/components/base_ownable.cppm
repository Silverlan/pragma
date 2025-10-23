// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.base_ownable;

export import :entities.components.base;
export import :entities.property;

export namespace pragma {
	struct DLLNETWORK CEOnOwnerChanged : public ComponentEvent {
		CEOnOwnerChanged(pragma::ecs::BaseEntity *oldOwner, pragma::ecs::BaseEntity *newOwner);
		virtual void PushArguments(lua_State *l) override;
		pragma::ecs::BaseEntity *oldOwner;
		pragma::ecs::BaseEntity *newOwner;
	};
	class DLLNETWORK BaseOwnableComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_OWNER_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		const pragma::PEntityProperty &GetOwnerProperty() const;
		void SetOwner(pragma::ecs::BaseEntity &owner);
		void ClearOwner();
		pragma::ecs::BaseEntity *GetOwner();
		const pragma::ecs::BaseEntity *GetOwner() const;
	  protected:
		BaseOwnableComponent(pragma::ecs::BaseEntity &ent);
		virtual void SetOwner(pragma::ecs::BaseEntity *owner);
		pragma::NetEventId m_netEvSetOwner = pragma::INVALID_NET_EVENT;
		pragma::PEntityProperty m_owner;
	};
};
