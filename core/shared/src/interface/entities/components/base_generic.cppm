// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_generic;

export import :entities.components.base;

export namespace pragma {
	namespace baseGenericComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_ENTITY_COMPONENT_ADDED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_ENTITY_COMPONENT_REMOVED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_MEMBERS_CHANGED)
	}
	class DLLNETWORK BaseGenericComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
	  protected:
		BaseGenericComponent(ecs::BaseEntity &ent);
	};
};
