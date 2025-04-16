/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"
#include "pragma/entities/components/s_point_at_target_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>

using namespace pragma;

void SPointAtTargetComponent::Initialize() { BasePointAtTargetComponent::Initialize(); }
void SPointAtTargetComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SPointAtTargetComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { nwm::write_unique_entity(packet, GetPointAtTarget()); }

void SPointAtTargetComponent::SetPointAtTarget(BaseEntity *ent)
{
	BasePointAtTargetComponent::SetPointAtTarget(ent);
	NetPacket p {};
	nwm::write_entity(p, ent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetPointAtTarget, p, pragma::networking::Protocol::SlowReliable);
}
