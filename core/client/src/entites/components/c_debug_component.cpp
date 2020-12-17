/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_debug_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

void CDebugTextComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	m_debugObject = DebugRenderer::DrawText(m_debugText,pos,m_size,color);
}
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
Bool CDebugTextComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetText)
		SetText(packet->ReadString());
	else if(eventId == m_netEvSetSize)
		SetSize(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}
void CDebugTextComponent::ReceiveData(NetPacket &packet)
{
	m_debugText = packet->ReadString();
	m_size = packet->Read<float>();
}

////////////////

void CBaseDebugOutlineComponent::ReceiveData(NetPacket &packet)
{
	auto *pDebugComponent = dynamic_cast<BaseDebugOutlineComponent*>(this);
	pDebugComponent->SetOutlineColor(packet->Read<Color>());
}

////////////////
	
void CDebugPointComponent::ReceiveData(NetPacket &packet)
{
	m_bAxis = packet->Read<bool>();
}
luabind::object CDebugPointComponent::InitializeLuaObject(lua_State *l) {return BaseDebugPointComponent::InitializeLuaObject<CDebugPointComponentHandleWrapper>(l);}
void CDebugPointComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	m_debugObject = DebugRenderer::DrawPoint(pos,color);
}

////////////////

void CDebugLineComponent::ReceiveData(NetPacket &packet)
{
	m_targetEntity = packet->ReadString();
	m_targetOrigin = packet->Read<Vector3>();
}
luabind::object CDebugLineComponent::InitializeLuaObject(lua_State *l) {return BaseDebugLineComponent::InitializeLuaObject<CDebugLineComponentHandleWrapper>(l);}
void CDebugLineComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	auto origin = m_targetOrigin;
	if(m_targetEntity.empty() == false)
	{
		EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(),EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
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
	m_debugObject = DebugRenderer::DrawLine(pos,origin,color);
}
	
////////////////

void CDebugBoxComponent::SetBounds(const Vector3 &min,const Vector3 &max)
{
	if(m_bounds.first == min && m_bounds.second == max)
		return;
	BaseDebugBoxComponent::SetBounds(min,max);
	ReloadDebugObject();
}
		
void CDebugBoxComponent::ReceiveData(NetPacket &packet)
{
	CBaseDebugOutlineComponent::ReceiveData(packet);
	m_bounds.first = packet->Read<Vector3>();
	m_bounds.second = packet->Read<Vector3>();
}
luabind::object CDebugBoxComponent::InitializeLuaObject(lua_State *l) {return BaseDebugBoxComponent::InitializeLuaObject<CDebugBoxComponentHandleWrapper>(l);}
void CDebugBoxComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto ang = pTrComponent != nullptr ? pTrComponent->GetAngles() : EulerAngles{};
	if(m_outlineColor.a > 0)
		m_debugObject = DebugRenderer::DrawBox(pos,m_bounds.first,m_bounds.second,ang,color,m_outlineColor);
	else
		m_debugObject = DebugRenderer::DrawBox(pos,m_bounds.first,m_bounds.second,ang,color);
}
Bool CDebugBoxComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetBounds)
	{
		auto min = packet->Read<Vector3>();
		auto max = packet->Read<Vector3>();
		SetBounds(min,max);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}

////////////////
	
void CDebugSphereComponent::ReceiveData(NetPacket &packet)
{
	CBaseDebugOutlineComponent::ReceiveData(packet);
	m_recursionLevel = packet->Read<uint32_t>();
}
luabind::object CDebugSphereComponent::InitializeLuaObject(lua_State *l) {return BaseDebugSphereComponent::InitializeLuaObject<CDebugSphereComponentHandleWrapper>(l);}
void CDebugSphereComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	if(pRadiusComponent.expired())
		return;
	if(m_outlineColor.a > 0)
		m_debugObject = DebugRenderer::DrawSphere(pos,pRadiusComponent->GetRadius(),color,m_outlineColor,0.f,m_recursionLevel);
	else
		m_debugObject = DebugRenderer::DrawSphere(pos,pRadiusComponent->GetRadius(),color,0.f,m_recursionLevel);
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
luabind::object CDebugConeComponent::InitializeLuaObject(lua_State *l) {return BaseDebugConeComponent::InitializeLuaObject<CDebugConeComponentHandleWrapper>(l);}
void CDebugConeComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	auto &ent = GetEntity();
	auto pRadiusComponent = ent.GetComponent<CRadiusComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	if(pRadiusComponent.expired() || pTrComponent == nullptr)
		return;
	if(m_startRadius > 0.f)
	{
		// Truncated Cone
		if(m_outlineColor.a > 0)
			m_debugObject = DebugRenderer::DrawTruncatedCone(pos,m_startRadius,pTrComponent->GetForward(),pRadiusComponent->GetRadius(),m_coneAngle,color,m_outlineColor,0.f,m_segmentCount);
		else
			m_debugObject = DebugRenderer::DrawTruncatedCone(pos,m_startRadius,pTrComponent->GetForward(),pRadiusComponent->GetRadius(),m_coneAngle,color,0.f,m_segmentCount);
		return;
	}
	if(m_outlineColor.a > 0)
		m_debugObject = DebugRenderer::DrawCone(pos,pTrComponent->GetForward(),pRadiusComponent->GetRadius(),m_coneAngle,color,m_outlineColor,0.f,m_segmentCount);
	else
		m_debugObject = DebugRenderer::DrawCone(pos,pTrComponent->GetForward(),pRadiusComponent->GetRadius(),m_coneAngle,color,0.f,m_segmentCount);
}
Bool CDebugConeComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetConeAngle)
		SetConeAngle(packet->Read<float>());
	else if(eventId == m_netEvSetStartRadius)
		SetStartRadius(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
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
luabind::object CDebugCylinderComponent::InitializeLuaObject(lua_State *l) {return BaseDebugCylinderComponent::InitializeLuaObject<CDebugCylinderComponentHandleWrapper>(l);}
void CDebugCylinderComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	auto &ent = GetEntity();
	auto pRadiusComponent = ent.GetComponent<CRadiusComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	if(pRadiusComponent.expired() || pTrComponent == nullptr)
		return;
	if(m_outlineColor.a > 0)
		m_debugObject = DebugRenderer::DrawCylinder(pos,pTrComponent->GetForward(),m_length,pRadiusComponent->GetRadius(),color,m_outlineColor,m_segmentCount);
	else
		m_debugObject = DebugRenderer::DrawCylinder(pos,pTrComponent->GetForward(),m_length,pRadiusComponent->GetRadius(),color,0.f,m_segmentCount);
}
Bool CDebugCylinderComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetLength)
		SetLength(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}

////////////////
	
luabind::object CDebugPlaneComponent::InitializeLuaObject(lua_State *l) {return BaseDebugPlaneComponent::InitializeLuaObject<CDebugPlaneComponentHandleWrapper>(l);}
void CDebugPlaneComponent::ReloadDebugObject(Color color,const Vector3 &pos)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	umath::Plane plane {pTrComponent->GetForward(),0.0};
	plane.MoveToPos(pos);
	m_debugObject = DebugRenderer::DrawPlane(plane,color);
}
void CDebugPlaneComponent::ReceiveData(NetPacket &packet) {}

