// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.debug;

import :entities;

using namespace pragma;

void SDebugTextComponent::SetText(const std::string &text)
{
	if(string::compare(m_debugText, text) == true)
		return;
	BaseDebugTextComponent::SetText(text);

	NetPacket packet {};
	packet->WriteString(text);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetText, packet, networking::Protocol::SlowReliable);
}
void SDebugTextComponent::SetSize(float size)
{
	if(m_size == size)
		return;
	BaseDebugTextComponent::SetSize(size);

	NetPacket packet {};
	packet->Write<float>(size);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetSize, packet, networking::Protocol::SlowReliable);
}
void SDebugTextComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SDebugTextComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_debugText);
	packet->Write<float>(m_size);
}

////////////////

void SBaseDebugOutlineComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	auto *pDebugComponent = dynamic_cast<BaseDebugOutlineComponent *>(this);
	packet->Write<Color>(pDebugComponent->GetOutlineColor());
}

////////////////

void SDebugPointComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<bool>(m_bAxis); }
void SDebugPointComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void SDebugLineComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_targetEntity);
	packet->Write<Vector3>(m_targetOrigin);
}
void SDebugLineComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void SDebugBoxComponent::SetBounds(const Vector3 &min, const Vector3 &max)
{
	if(m_bounds.first == min && m_bounds.second == max)
		return;
	BaseDebugBoxComponent::SetBounds(min, max);

	NetPacket packet {};
	packet->Write<Vector3>(min);
	packet->Write<Vector3>(max);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetBounds, packet, networking::Protocol::SlowReliable);
}

void SDebugBoxComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet, rp);
	packet->Write<Vector3>(m_bounds.first);
	packet->Write<Vector3>(m_bounds.second);
}
void SDebugBoxComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void SDebugSphereComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet, rp);
	packet->Write<uint32_t>(m_recursionLevel);
}
void SDebugSphereComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void SDebugConeComponent::SetConeAngle(float angle)
{
	if(angle == m_coneAngle)
		return;
	BaseDebugConeComponent::SetConeAngle(angle);

	NetPacket packet {};
	packet->Write<float>(angle);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetConeAngle, packet, networking::Protocol::SlowReliable);
}
void SDebugConeComponent::SetStartRadius(float radius)
{
	if(radius == m_startRadius)
		return;
	BaseDebugConeComponent::SetStartRadius(radius);

	NetPacket packet {};
	packet->Write<float>(radius);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetStartRadius, packet, networking::Protocol::SlowReliable);
}

void SDebugConeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet, rp);
	packet->Write<float>(m_coneAngle);
	packet->Write<float>(m_startRadius);
	packet->Write<uint32_t>(m_segmentCount);
}
void SDebugConeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void SDebugCylinderComponent::SetLength(float length)
{
	if(length == m_length)
		return;
	BaseDebugCylinderComponent::SetLength(length);

	NetPacket packet {};
	packet->Write<float>(length);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetLength, packet, networking::Protocol::SlowReliable);
}

void SDebugCylinderComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet, rp);
	packet->Write<float>(m_length);
	packet->Write<uint32_t>(m_segmentCount);
}
void SDebugCylinderComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void SDebugPlaneComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
