// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.liquid.base_volume;

using namespace pragma;

void BaseLiquidVolumeComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}

void BaseLiquidVolumeComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
BaseLiquidVolumeComponent::BaseLiquidVolumeComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidVolumeComponent::Initialize() { BaseEntityComponent::Initialize(); }
