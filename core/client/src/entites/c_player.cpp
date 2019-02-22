#include "stdafx_client.h"
#include "pragma/entities/c_player.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

LINK_ENTITY_TO_CLASS(player,CPlayer);

void CPlayer::Initialize()
{
	CBaseEntity::Initialize();
	auto whPlComponent = AddComponent<pragma::CPlayerComponent>();
	if(whPlComponent.expired() == false)
		m_basePlayerComponent = util::WeakHandle<pragma::BasePlayerComponent>(std::static_pointer_cast<pragma::BasePlayerComponent>(whPlComponent.get()->shared_from_this()));
}
