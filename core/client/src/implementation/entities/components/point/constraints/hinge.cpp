// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_constraint_hinge;
using namespace pragma;

void CPointConstraintHingeComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = networking::read_vector(packet);

	m_kvLimitLow = packet->Read<float>();
	m_kvLimitHigh = packet->Read<float>();
	m_kvLimitSoftness = packet->Read<float>();
	m_kvLimitBiasFactor = packet->Read<float>();
	m_kvLimitRelaxationFactor = packet->Read<float>();
}
void CPointConstraintHingeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointConstraintHinge::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintHingeComponent>();
}
