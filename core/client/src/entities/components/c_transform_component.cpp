/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <stack>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

void CTransformComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CTransformComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseTransformComponent)); }
void CTransformComponent::ReceiveData(NetPacket &packet)
{
	Vector3 pos = nwm::read_vector(packet);
	auto rot = nwm::read_quat(packet);
	SetPosition(pos);
	SetRotation(rot);
	SetEyeOffset(packet->Read<Vector3>());

	auto scale = packet->Read<Vector3>();
	SetScale(scale);
}

Bool CTransformComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetScale)
		SetScale(packet->Read<Vector3>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
