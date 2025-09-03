// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.point.constraints.cone_twist;

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_conetwist, PointConstraintConeTwist);

void SPointConstraintConeTwistComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	nwm::write_vector(packet, m_posTarget);

	packet->Write<float>(m_kvSwingSpan1);
	packet->Write<float>(m_kvSwingSpan2);
	packet->Write<float>(m_kvTwistSpan);
	packet->Write<float>(m_kvSoftness);
	packet->Write<float>(m_kvRelaxationFactor);
}

void SPointConstraintConeTwistComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointConstraintConeTwist::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintConeTwistComponent>();
}
