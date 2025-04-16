/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/audio/alsound.h>
#include "pragma/audio/s_alsound.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/components/map_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/netpacket.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound, EnvSound);

void SSoundComponent::Initialize() { BaseEnvSoundComponent::Initialize(); }
void SSoundComponent::OnSoundCreated(ALSound &snd)
{
	BaseEnvSoundComponent::OnSoundCreated(snd);
	auto pMapComponent = GetEntity().GetComponent<pragma::MapComponent>();
	dynamic_cast<SALSound &>(snd).SetEntityMapIndex(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
}
void SSoundComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvMaxDist);
	auto idx = (m_sound != nullptr) ? m_sound->GetIndex() : std::numeric_limits<uint32_t>::max();
	packet->Write<uint32_t>(idx);
}

void SSoundComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void EnvSound::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundComponent>();
}
