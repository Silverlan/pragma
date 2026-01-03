// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_constraint_cone_twist;
using namespace pragma;

void CPointConstraintConeTwistComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = networking::read_vector(packet);

	m_kvSwingSpan1 = packet->Read<float>();
	m_kvSwingSpan2 = packet->Read<float>();
	m_kvTwistSpan = packet->Read<float>();
	m_kvSoftness = packet->Read<float>();
	// m_kvBiasFactor = packet->Read<float>();
	m_kvRelaxationFactor = packet->Read<float>();
}
void CPointConstraintConeTwistComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointConstraintConeTwist::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintConeTwistComponent>();
}
