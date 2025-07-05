// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/components/liquid/base_liquid_volume_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"

using namespace pragma;

void BaseLiquidVolumeComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}

void BaseLiquidVolumeComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
BaseLiquidVolumeComponent::BaseLiquidVolumeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidVolumeComponent::Initialize() { BaseEntityComponent::Initialize(); }
