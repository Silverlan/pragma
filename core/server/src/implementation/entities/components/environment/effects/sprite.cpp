// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.effects.sprite;

using namespace pragma;

void SSpriteComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_spritePath);
	packet->Write<float>(m_size);
	packet->Write<float>(m_bloomScale);
	packet->Write<Color>(m_color);
	packet->Write<uint32_t>(m_particleRenderMode);
	packet->Write<float>(m_tFadeIn);
	packet->Write<float>(m_tFadeOut);
}

void SSpriteComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvSprite::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSpriteComponent>();
}
