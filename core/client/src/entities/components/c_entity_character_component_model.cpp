/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_entity_character_component_model.hpp"

using namespace pragma;

CEntityCharacterComponentModel::CEntityCharacterComponentModel(CBaseEntity &ent)
	: BaseEntityComponent(ent)
{}

void CEntityCharacterComponentModel::Initialize()
{
	BaseEntityComponent::Initialize();
}
