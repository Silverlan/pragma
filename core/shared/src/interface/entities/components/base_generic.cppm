// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_generic;

export import :entities.components.base;

export namespace pragma {
	namespace baseGenericComponent {
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_ENTITY_COMPONENT_ADDED;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_ENTITY_COMPONENT_REMOVED;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_MEMBERS_CHANGED;
	}
	class DLLNETWORK BaseGenericComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
	  protected:
		BaseGenericComponent(pragma::ecs::BaseEntity &ent);
	};
};
