// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.point_constraint_dof;

using namespace pragma;

void CPointConstraintDoFComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);

	m_kvLimLinLower = nwm::read_vector(packet);
	m_kvLimLinUpper = nwm::read_vector(packet);
	m_kvLimAngLower = nwm::read_vector(packet);
	m_kvLimAngUpper = nwm::read_vector(packet);
}
void CPointConstraintDoFComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void CPointConstraintDoF::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintDoFComponent>();
}
