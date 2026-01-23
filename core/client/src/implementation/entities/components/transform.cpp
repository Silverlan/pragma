// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.transform;
using namespace pragma;

void CTransformComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CTransformComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseTransformComponent)); }
void CTransformComponent::ReceiveData(NetPacket &packet)
{
	Vector3 pos = networking::read_vector(packet);
	auto rot = networking::read_quat(packet);
	SetPosition(pos);
	SetRotation(rot);
	SetEyeOffset(packet->Read<Vector3>());

	auto scale = packet->Read<Vector3>();
	SetScale(scale);
}

Bool CTransformComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetScale)
		SetScale(packet->Read<Vector3>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
