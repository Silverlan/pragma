/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_generic_component.hpp"

using namespace pragma;

decltype(BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_ADDED) BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_ADDED = INVALID_COMPONENT_ID;
decltype(BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED) BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED = INVALID_COMPONENT_ID;
decltype(BaseGenericComponent::EVENT_ON_MEMBERS_CHANGED) BaseGenericComponent::EVENT_ON_MEMBERS_CHANGED = INVALID_COMPONENT_ID;
void BaseGenericComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_ENTITY_COMPONENT_ADDED = registerEvent("GENERIC_ON_ENTITY_COMPONENT_ADDED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_ENTITY_COMPONENT_REMOVED = registerEvent("GENERIC_ON_ENTITY_COMPONENT_REMOVED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_MEMBERS_CHANGED = registerEvent("GENERIC_ON_MEMBERS_CHANGED", ComponentEventInfo::Type::Explicit);
}

BaseGenericComponent::BaseGenericComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
