// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/point/constraints/c_point_constraint_ballsocket.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_ballsocket, CPointConstraintBallSocket);

void CPointConstraintBallSocketComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);
}
void CPointConstraintBallSocketComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointConstraintBallSocket::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintBallSocketComponent>();
}
