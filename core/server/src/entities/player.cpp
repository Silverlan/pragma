#include "stdafx_server.h"
#include "pragma/entities/player.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"

LINK_ENTITY_TO_CLASS(player,Player);

void Player::Initialize()
{
	SBaseEntity::Initialize();
	auto whPlComponent = AddComponent<pragma::SPlayerComponent>();
	if(whPlComponent.expired() == false)
		m_basePlayerComponent = util::WeakHandle<pragma::BasePlayerComponent>(std::static_pointer_cast<pragma::BasePlayerComponent>(whPlComponent.get()->shared_from_this()));
}
