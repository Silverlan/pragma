// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_constraint_dof;
using namespace pragma;

void CPointConstraintDoFComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = networking::read_vector(packet);

	m_kvLimLinLower = networking::read_vector(packet);
	m_kvLimLinUpper = networking::read_vector(packet);
	m_kvLimAngLower = networking::read_vector(packet);
	m_kvLimAngUpper = networking::read_vector(packet);
}
void CPointConstraintDoFComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void CPointConstraintDoF::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintDoFComponent>();
}
