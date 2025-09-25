// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/components/liquid/base_liquid_surface_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"

export module pragma.shared;

import :entities.components.constraints.liquid_surface;

using namespace pragma;

void BaseLiquidSurfaceComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}

void BaseLiquidSurfaceComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
BaseLiquidSurfaceComponent::BaseLiquidSurfaceComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidSurfaceComponent::Initialize() { BaseEntityComponent::Initialize(); }
