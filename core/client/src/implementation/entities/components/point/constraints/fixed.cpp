// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_constraint_fixed;
using namespace pragma;

void CPointConstraintFixedComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = networking::read_vector(packet);
}
void CPointConstraintFixedComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointConstraintFixed::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintFixedComponent>();
}
