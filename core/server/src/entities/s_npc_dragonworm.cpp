// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/s_npc_dragonworm.h"
#include <pragma/entities/components/base_model_component.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void SDragonWormComponent::Initialize() { BaseEntityComponent::Initialize(); }

void SDragonWormComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto mdlComponent = GetEntity().GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetModel("creatures/dragonworm.wmd");
}
