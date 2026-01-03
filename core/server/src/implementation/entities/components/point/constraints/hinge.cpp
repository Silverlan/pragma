// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point.constraints.hinge;

using namespace pragma;

void SPointConstraintHingeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	networking::write_vector(packet, m_posTarget);

	packet->Write<float>(m_kvLimitLow);
	packet->Write<float>(m_kvLimitHigh);
	packet->Write<float>(m_kvLimitSoftness);
	packet->Write<float>(m_kvLimitBiasFactor);
	packet->Write<float>(m_kvLimitRelaxationFactor);
}

void SPointConstraintHingeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointConstraintHinge::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintHingeComponent>();
}
