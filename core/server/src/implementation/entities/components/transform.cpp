// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.transform;

import :entities;
import :server_state;

using namespace pragma;

void STransformComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseTransformComponent)); }
void STransformComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	networking::write_vector(packet, GetPosition());
	networking::write_quat(packet, GetRotation());
	packet->Write<Vector3>(GetEyeOffset());
	packet->Write<Vector3>(GetScale());
}
void STransformComponent::SetScale(const Vector3 &scale)
{
	if(scale == GetScale())
		return;
	BaseTransformComponent::SetScale(scale);
	NetPacket p;
	p->Write<Vector3>(scale);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	ent.SendNetEvent(m_netEvSetScale, p, networking::Protocol::SlowReliable);
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(pPhysComponent->GetPhysicsType());
}
void STransformComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void STransformComponent::SetEyeOffset(const Vector3 &offset)
{
	BaseTransformComponent::SetEyeOffset(offset);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	networking::write_vector(p, offset);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_EYEOFFSET, p, networking::Protocol::SlowReliable);
}
