// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/components/base_game_component.hpp"

using namespace pragma;

void BaseGameComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}
BaseGameComponent::BaseGameComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseGameComponent::Initialize() { BaseEntityComponent::Initialize(); }

void BaseGameComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

void BaseGameComponent::Save(udm::LinkedPropertyWrapperArg udm) { BaseEntityComponent::Save(udm); }

void BaseGameComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) { BaseEntityComponent::Load(udm, version); }
