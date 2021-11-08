/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_game_component.hpp"

using namespace pragma;

void BaseGameComponent::RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent)
{
}
BaseGameComponent::BaseGameComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}

void BaseGameComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}

void BaseGameComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
}

void BaseGameComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
}

void BaseGameComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
}
