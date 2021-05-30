/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_OWNABLE_COMPONENT_HPP__
#define __BASE_OWNABLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include "pragma/entities/entity_property.hpp"

namespace pragma
{
	struct DLLNETWORK CEOnOwnerChanged
		: public ComponentEvent
	{
		CEOnOwnerChanged(BaseEntity *oldOwner,BaseEntity *newOwner);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity *oldOwner;
		BaseEntity *newOwner;
	};
	class DLLNETWORK BaseOwnableComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_OWNER_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		virtual void Initialize() override;
		const pragma::PEntityProperty &GetOwnerProperty() const;
		void SetOwner(BaseEntity &owner);
		void ClearOwner();
		BaseEntity *GetOwner();
		const BaseEntity *GetOwner() const;
	protected:
		BaseOwnableComponent(BaseEntity &ent);
		virtual void SetOwner(BaseEntity *owner);
		pragma::NetEventId m_netEvSetOwner = pragma::INVALID_NET_EVENT;
		pragma::PEntityProperty m_owner;
	};
};

#endif
