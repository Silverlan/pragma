// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.base_generic;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseGenericComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static ComponentEventId EVENT_ON_ENTITY_COMPONENT_ADDED;
		static ComponentEventId EVENT_ON_ENTITY_COMPONENT_REMOVED;
		static ComponentEventId EVENT_ON_MEMBERS_CHANGED;
	  protected:
		BaseGenericComponent(BaseEntity &ent);
	};
};
