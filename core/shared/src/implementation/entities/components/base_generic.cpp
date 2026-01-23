// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_generic;

using namespace pragma;

void BaseGenericComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseGenericComponent::EVENT_ON_ENTITY_COMPONENT_ADDED = registerEvent("GENERIC_ON_ENTITY_COMPONENT_ADDED", ComponentEventInfo::Type::Explicit);
	baseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED = registerEvent("GENERIC_ON_ENTITY_COMPONENT_REMOVED", ComponentEventInfo::Type::Explicit);
	baseGenericComponent::EVENT_ON_MEMBERS_CHANGED = registerEvent("GENERIC_ON_MEMBERS_CHANGED", ComponentEventInfo::Type::Explicit);
}

BaseGenericComponent::BaseGenericComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
