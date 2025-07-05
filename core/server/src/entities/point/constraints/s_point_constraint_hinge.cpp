// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/point/constraints/s_point_constraint_hinge.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_hinge, PointConstraintHinge);

void SPointConstraintHingeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	nwm::write_vector(packet, m_posTarget);

	packet->Write<float>(m_kvLimitLow);
	packet->Write<float>(m_kvLimitHigh);
	packet->Write<float>(m_kvLimitSoftness);
	packet->Write<float>(m_kvLimitBiasFactor);
	packet->Write<float>(m_kvLimitRelaxationFactor);
}

void SPointConstraintHingeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointConstraintHinge::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintHingeComponent>();
}
