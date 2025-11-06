// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.client;

import :entities.components.debug;
import :debug;
import :engine;
import :entities.components.radius;

using namespace pragma;

void CDebugTextComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo) { m_debugObject = DebugRenderer::DrawText(renderInfo, m_debugText, m_size); }
void CDebugTextComponent::SetText(const std::string &text)
{
	BaseDebugTextComponent::SetText(text);
	if(GetEntity().IsSpawned())
		ReloadDebugObject();
}
void CDebugTextComponent::SetSize(float size)
{
	BaseDebugTextComponent::SetSize(size);
	if(GetEntity().IsSpawned())
		ReloadDebugObject();
}
Bool CDebugTextComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetText)
		SetText(packet->ReadString());
	else if(eventId == m_netEvSetSize)
		SetSize(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
void CDebugTextComponent::ReceiveData(NetPacket &packet)
{
	m_debugText = packet->ReadString();
	m_size = packet->Read<float>();
}
void CDebugTextComponent::InitializeLuaObject(lua::State *l) { return TCBaseDebugComponent<BaseDebugTextComponent>::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void CBaseDebugOutlineComponent::ReceiveData(NetPacket &packet)
{
	auto *pDebugComponent = dynamic_cast<BaseDebugOutlineComponent *>(this);
	pDebugComponent->SetOutlineColor(packet->Read<Color>());
}

////////////////

void CDebugPointComponent::ReceiveData(NetPacket &packet) { m_bAxis = packet->Read<bool>(); }
void CDebugPointComponent::InitializeLuaObject(lua::State *l) { return BaseDebugPointComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugPointComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo) { m_debugObject = DebugRenderer::DrawPoint(renderInfo); }

////////////////

void CDebugLineComponent::ReceiveData(NetPacket &packet)
{
	m_targetEntity = packet->ReadString();
	m_targetOrigin = packet->Read<Vector3>();
}
void CDebugLineComponent::InitializeLuaObject(lua::State *l) { return BaseDebugLineComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugLineComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo)
{
	auto origin = m_targetOrigin;
	if(m_targetEntity.empty() == false) {
		pragma::ecs::EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(), pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterName>(m_targetEntity);
		auto it = entIt.begin();
		if(it == entIt.end())
			return;
		auto *ent = *it;
		auto trComponent = (ent != nullptr) ? ent->GetTransformComponent() : nullptr;
		if(!trComponent)
			return;
		origin = trComponent->GetPosition();
	}
	renderInfo.SetOrigin({});
	m_debugObject = DebugRenderer::DrawLine(pos, origin, renderInfo);
}

////////////////

void CDebugBoxComponent::SetBounds(const Vector3 &min, const Vector3 &max)
{
	if(m_bounds.first == min && m_bounds.second == max)
		return;
	BaseDebugBoxComponent::SetBounds(min, max);
	ReloadDebugObject();
}

void CDebugBoxComponent::ReceiveData(NetPacket &packet)
{
	CBaseDebugOutlineComponent::ReceiveData(packet);
	m_bounds.first = packet->Read<Vector3>();
	m_bounds.second = packet->Read<Vector3>();
}
void CDebugBoxComponent::InitializeLuaObject(lua::State *l) { return BaseDebugBoxComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugBoxComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto ang = pTrComponent != nullptr ? pTrComponent->GetAngles() : EulerAngles {};
	if(m_outlineColor.a > 0)
		renderInfo.SetOutlineColor(m_outlineColor);
	renderInfo.SetRotation(uquat::create(ang));
	m_debugObject = DebugRenderer::DrawBox(m_bounds.first, m_bounds.second, renderInfo);
}
Bool CDebugBoxComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetBounds) {
		auto min = packet->Read<Vector3>();
		auto max = packet->Read<Vector3>();
		SetBounds(min, max);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

////////////////

void CDebugSphereComponent::ReceiveData(NetPacket &packet)
{
	CBaseDebugOutlineComponent::ReceiveData(packet);
	m_recursionLevel = packet->Read<uint32_t>();
}
void CDebugSphereComponent::InitializeLuaObject(lua::State *l) { return BaseDebugSphereComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugSphereComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo)
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	if(pRadiusComponent.expired())
		return;
	if(m_outlineColor.a > 0)
		renderInfo.SetOutlineColor(m_outlineColor);
	m_debugObject = DebugRenderer::DrawSphere(renderInfo, pRadiusComponent->GetRadius(), m_recursionLevel);
}

////////////////

void CDebugConeComponent::SetConeAngle(float angle)
{
	if(angle == m_coneAngle)
		return;
	BaseDebugConeComponent::SetConeAngle(angle);
	ReloadDebugObject();
}
void CDebugConeComponent::SetStartRadius(float radius)
{
	if(radius == m_startRadius)
		return;
	BaseDebugConeComponent::SetStartRadius(radius);
	ReloadDebugObject();
}

void CDebugConeComponent::ReceiveData(NetPacket &packet)
{
	CBaseDebugOutlineComponent::ReceiveData(packet);
	m_coneAngle = packet->Read<float>();
	m_startRadius = packet->Read<float>();
	m_segmentCount = packet->Read<uint32_t>();
}
void CDebugConeComponent::InitializeLuaObject(lua::State *l) { return BaseDebugConeComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugConeComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo)
{
	auto &ent = GetEntity();
	auto pRadiusComponent = ent.GetComponent<CRadiusComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	if(pRadiusComponent.expired() || pTrComponent == nullptr)
		return;
	if(m_outlineColor.a > 0)
		renderInfo.SetOutlineColor(m_outlineColor);
	if(m_startRadius > 0.f) {
		// Truncated Cone
		m_debugObject = DebugRenderer::DrawTruncatedCone(renderInfo, m_startRadius, pTrComponent->GetForward(), pRadiusComponent->GetRadius(), m_coneAngle, m_segmentCount);
		return;
	}
	m_debugObject = DebugRenderer::DrawCone(renderInfo, pTrComponent->GetForward(), pRadiusComponent->GetRadius(), m_coneAngle, m_segmentCount);
}
Bool CDebugConeComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetConeAngle)
		SetConeAngle(packet->Read<float>());
	else if(eventId == m_netEvSetStartRadius)
		SetStartRadius(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

////////////////

void CDebugCylinderComponent::SetLength(float length)
{
	if(length == m_length)
		return;
	BaseDebugCylinderComponent::SetLength(length);

	ReloadDebugObject();
}

void CDebugCylinderComponent::ReceiveData(NetPacket &packet)
{
	CBaseDebugOutlineComponent::ReceiveData(packet);
	m_length = packet->Read<float>();
	m_segmentCount = packet->Read<uint32_t>();
}
void CDebugCylinderComponent::InitializeLuaObject(lua::State *l) { return BaseDebugCylinderComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugCylinderComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo)
{
	auto &ent = GetEntity();
	auto pRadiusComponent = ent.GetComponent<CRadiusComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	if(pRadiusComponent.expired() || pTrComponent == nullptr)
		return;
	if(m_outlineColor.a > 0)
		renderInfo.SetOutlineColor(m_outlineColor);
	m_debugObject = DebugRenderer::DrawCylinder(renderInfo, pTrComponent->GetForward(), m_length, pRadiusComponent->GetRadius(), m_segmentCount);
}
Bool CDebugCylinderComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetLength)
		SetLength(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

////////////////

void CDebugPlaneComponent::InitializeLuaObject(lua::State *l) { return BaseDebugPlaneComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugPlaneComponent::DoReloadDebugObject(Color color, const Vector3 &pos, DebugRenderInfo renderInfo)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	umath::Plane plane {pTrComponent->GetForward(), 0.0};
	plane.MoveToPos(pos);
	m_debugObject = DebugRenderer::DrawPlane(plane, color);
}
void CDebugPlaneComponent::ReceiveData(NetPacket &packet) {}
