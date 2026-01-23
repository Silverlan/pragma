// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.liquid.base_surface;

using namespace pragma;

void BaseLiquidSurfaceComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}

void BaseLiquidSurfaceComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
BaseLiquidSurfaceComponent::BaseLiquidSurfaceComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidSurfaceComponent::Initialize() { BaseEntityComponent::Initialize(); }
