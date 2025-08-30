// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseplayer.hpp>
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

export module pragma.client.entities:player;

import pragma.entities.components;

export class DLLCLIENT CPlayer : public CBaseEntity, public BasePlayer {
  public:
	virtual void Initialize() override;
};

LINK_ENTITY_TO_CLASS(player, CPlayer);

void CPlayer::Initialize()
{
	CBaseEntity::Initialize();
	auto whPlComponent = AddComponent<pragma::CPlayerComponent>();
	if(whPlComponent.expired() == false)
		m_basePlayerComponent = util::WeakHandle<pragma::BasePlayerComponent>(std::static_pointer_cast<pragma::BasePlayerComponent>(whPlComponent.get()->shared_from_this()));
}
