// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/components/c_entity_character_component_model.hpp"
#include "pragma/entities/c_baseentity.h"

using namespace pragma;

CEntityCharacterComponentModel::CEntityCharacterComponentModel(CBaseEntity &ent) : BaseEntityComponent(ent) {}

void CEntityCharacterComponentModel::Initialize() { BaseEntityComponent::Initialize(); }
