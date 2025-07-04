// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/point/constraints/c_point_constraint_hinge.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_hinge, CPointConstraintHinge);

void CPointConstraintHingeComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);

	m_kvLimitLow = packet->Read<float>();
	m_kvLimitHigh = packet->Read<float>();
	m_kvLimitSoftness = packet->Read<float>();
	m_kvLimitBiasFactor = packet->Read<float>();
	m_kvLimitRelaxationFactor = packet->Read<float>();
}
void CPointConstraintHingeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointConstraintHinge::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintHingeComponent>();
}
