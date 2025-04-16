/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/point/constraints/s_point_constraint_dof.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_dof, PointConstraintDoF);

void SPointConstraintDoFComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	nwm::write_vector(packet, m_posTarget);

	nwm::write_vector(packet, m_kvLimLinLower);
	nwm::write_vector(packet, m_kvLimLinUpper);
	nwm::write_vector(packet, m_kvLimAngLower);
	nwm::write_vector(packet, m_kvLimAngUpper);
}

void SPointConstraintDoFComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointConstraintDoF::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintDoFComponent>();
}
