// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point.constraints.cone_twist;

using namespace pragma;

void SPointConstraintConeTwistComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	networking::write_vector(packet, m_posTarget);

	packet->Write<float>(m_kvSwingSpan1);
	packet->Write<float>(m_kvSwingSpan2);
	packet->Write<float>(m_kvTwistSpan);
	packet->Write<float>(m_kvSoftness);
	packet->Write<float>(m_kvRelaxationFactor);
}

void SPointConstraintConeTwistComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointConstraintConeTwist::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintConeTwistComponent>();
}
