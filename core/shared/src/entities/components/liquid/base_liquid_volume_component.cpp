/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/liquid/base_liquid_volume_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"

using namespace pragma;

void BaseLiquidVolumeComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}

void BaseLiquidVolumeComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
BaseLiquidVolumeComponent::BaseLiquidVolumeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidVolumeComponent::Initialize() { BaseEntityComponent::Initialize(); }
