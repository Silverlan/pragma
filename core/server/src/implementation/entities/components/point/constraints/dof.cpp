// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point.constraints.dof;

using namespace pragma;

void SPointConstraintDoFComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	networking::write_vector(packet, m_posTarget);

	networking::write_vector(packet, m_kvLimLinLower);
	networking::write_vector(packet, m_kvLimLinUpper);
	networking::write_vector(packet, m_kvLimAngLower);
	networking::write_vector(packet, m_kvLimAngUpper);
}

void SPointConstraintDoFComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointConstraintDoF::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintDoFComponent>();
}
