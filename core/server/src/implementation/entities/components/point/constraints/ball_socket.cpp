// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point.constraints.ball_socket;

using namespace pragma;

void SPointConstraintBallSocketComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	networking::write_vector(packet, m_posTarget);
}

void SPointConstraintBallSocketComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////

void PointConstraintBallSocket::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintBallSocketComponent>();
}
