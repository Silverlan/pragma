// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_client.h"

module pragma.client;


import :entities.components.point_constraint_fixed;
using namespace pragma;

void CPointConstraintFixedComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);
}
void CPointConstraintFixedComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointConstraintFixed::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintFixedComponent>();
}
