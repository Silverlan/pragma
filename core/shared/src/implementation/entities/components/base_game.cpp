// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_game;

using namespace pragma;

void BaseGameComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}
BaseGameComponent::BaseGameComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseGameComponent::Initialize() { BaseEntityComponent::Initialize(); }

void BaseGameComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

void BaseGameComponent::Save(udm::LinkedPropertyWrapperArg udm) { BaseEntityComponent::Save(udm); }

void BaseGameComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) { BaseEntityComponent::Load(udm, version); }
