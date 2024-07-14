/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __BASE_CHILD_COMPONENT_HPP__
#define __BASE_CHILD_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/parentmode.h"
#include "pragma/entities/parentinfo.h"
#include "pragma/entities/entity_uuid_ref.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma {
	class ParentComponent;
	class DLLNETWORK BaseChildComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_PARENT_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		void ClearParent();
		void SetParent(const pragma::EntityURef &parent);
		const pragma::EntityURef &GetParent() const;

		BaseEntity *GetParentEntity();
		const BaseEntity *GetParentEntity() const { return const_cast<BaseChildComponent *>(this)->GetParentEntity(); }
		bool HasParent() const;
	  protected:
		BaseChildComponent(BaseEntity &ent);
		virtual void OnEntitySpawn() override;
		virtual void OnParentChanged(BaseEntity *parent) {};

		EntityURef m_parent;
		bool m_parentValid = false;
		pragma::NetEventId m_netEvSetParent = pragma::INVALID_NET_EVENT;
	};
};

#endif
