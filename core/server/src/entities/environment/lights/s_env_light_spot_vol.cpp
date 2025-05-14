/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"
#include "pragma/entities/environment/lights/s_env_light_spot_vol.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/networking/enums.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_spot_vol, EnvLightSpotVol);

void SLightSpotVolComponent::Initialize()
{
	BaseEnvLightSpotVolComponent::Initialize();
	static_cast<SBaseEntity &>(GetEntity()).SetSynchronized(false);
}
void SLightSpotVolComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_coneStartOffset);
	nwm::write_unique_entity(packet, m_hSpotlightTarget.get());
}

void SLightSpotVolComponent::SetSpotlightTarget(BaseEntity &ent)
{
	BaseEnvLightSpotVolComponent::SetSpotlightTarget(ent);
	NetPacket p {};
	nwm::write_entity(p, &ent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetSpotlightTarget, p, pragma::networking::Protocol::SlowReliable);
}

void SLightSpotVolComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvLightSpotVol::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightSpotVolComponent>();
}
