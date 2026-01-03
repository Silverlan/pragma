// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.decal;

using namespace pragma;

void SDecalComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SDecalComponent::Initialize()
{
	BaseEnvDecalComponent::Initialize();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	ent.SetSynchronized(false);
}

void SDecalComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_material);
	packet->Write<float>(m_size);
	packet->Write<bool>(m_startDisabled);
}

void EnvDecal::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDecalComponent>();
}
