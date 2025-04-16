/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_player.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

LINK_ENTITY_TO_CLASS(player, CPlayer);

void CPlayer::Initialize()
{
	CBaseEntity::Initialize();
	auto whPlComponent = AddComponent<pragma::CPlayerComponent>();
	if(whPlComponent.expired() == false)
		m_basePlayerComponent = util::WeakHandle<pragma::BasePlayerComponent>(std::static_pointer_cast<pragma::BasePlayerComponent>(whPlComponent.get()->shared_from_this()));
}
