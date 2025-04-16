/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/point/constraints/c_point_constraint_conetwist.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_conetwist, CPointConstraintConeTwist);

void CPointConstraintConeTwistComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);

	m_kvSwingSpan1 = packet->Read<float>();
	m_kvSwingSpan2 = packet->Read<float>();
	m_kvTwistSpan = packet->Read<float>();
	m_kvSoftness = packet->Read<float>();
	// m_kvBiasFactor = packet->Read<float>();
	m_kvRelaxationFactor = packet->Read<float>();
}
void CPointConstraintConeTwistComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointConstraintConeTwist::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintConeTwistComponent>();
}
