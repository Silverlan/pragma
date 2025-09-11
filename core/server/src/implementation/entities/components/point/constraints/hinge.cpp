// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.point.constraints.hinge;

using namespace pragma;

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
