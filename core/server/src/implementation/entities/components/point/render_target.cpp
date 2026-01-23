// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point.render_target;

using namespace pragma;

void SRenderTargetComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvMaterial);
	packet->Write<float>(m_kvFOV);
	packet->Write<float>(m_kvRefreshRate);
	packet->Write<float>(m_kvRenderWidth);
	packet->Write<float>(m_kvRenderHeight);
	packet->Write<float>(m_kvNearZ);
	packet->Write<float>(m_kvFarZ);
	packet->Write<int>(m_kvRenderDepth);
}

void SRenderTargetComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointRenderTarget::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SRenderTargetComponent>();
}
