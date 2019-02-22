#include "stdafx_server.h"
#include "pragma/entities/components/s_debug_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

void SDebugTextComponent::SetText(const std::string &text)
{
	if(ustring::compare(m_debugText,text) == true)
		return;
	BaseDebugTextComponent::SetText(text);

	NetPacket packet {};
	packet->WriteString(text);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetText,packet);
}
void SDebugTextComponent::SetSize(float size)
{
	if(m_size == size)
		return;
	BaseDebugTextComponent::SetSize(size);

	NetPacket packet {};
	packet->Write<float>(size);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetSize,packet);
}
luabind::object SDebugTextComponent::InitializeLuaObject(lua_State *l) {return BaseDebugTextComponent::InitializeLuaObject<SDebugTextComponentHandleWrapper>(l);}
void SDebugTextComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->WriteString(m_debugText);
	packet->Write<float>(m_size);
}

////////////////

void SBaseDebugOutlineComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	auto *pDebugComponent = dynamic_cast<BaseDebugOutlineComponent*>(this);
	packet->Write<Color>(pDebugComponent->GetOutlineColor());
}

////////////////
	
void SDebugPointComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<bool>(m_bAxis);
}
luabind::object SDebugPointComponent::InitializeLuaObject(lua_State *l) {return BaseDebugPointComponent::InitializeLuaObject<SDebugPointComponentHandleWrapper>(l);}

////////////////

void SDebugLineComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->WriteString(m_targetEntity);
	packet->Write<Vector3>(m_targetOrigin);
}
luabind::object SDebugLineComponent::InitializeLuaObject(lua_State *l) {return BaseDebugLineComponent::InitializeLuaObject<SDebugLineComponentHandleWrapper>(l);}
	
////////////////

void SDebugBoxComponent::SetBounds(const Vector3 &min,const Vector3 &max)
{
	if(m_bounds.first == min && m_bounds.second == max)
		return;
	BaseDebugBoxComponent::SetBounds(min,max);

	NetPacket packet {};
	packet->Write<Vector3>(min);
	packet->Write<Vector3>(max);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetBounds,packet);
}
		
void SDebugBoxComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet,rp);
	packet->Write<Vector3>(m_bounds.first);
	packet->Write<Vector3>(m_bounds.second);
}
luabind::object SDebugBoxComponent::InitializeLuaObject(lua_State *l) {return BaseDebugBoxComponent::InitializeLuaObject<SDebugBoxComponentHandleWrapper>(l);}

////////////////
	
void SDebugSphereComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet,rp);
	packet->Write<uint32_t>(m_recursionLevel);
}
luabind::object SDebugSphereComponent::InitializeLuaObject(lua_State *l) {return BaseDebugSphereComponent::InitializeLuaObject<SDebugSphereComponentHandleWrapper>(l);}

////////////////
	
void SDebugConeComponent::SetConeAngle(float angle)
{
	if(angle == m_coneAngle)
		return;
	BaseDebugConeComponent::SetConeAngle(angle);

	NetPacket packet {};
	packet->Write<float>(angle);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetConeAngle,packet);
}
void SDebugConeComponent::SetStartRadius(float radius)
{
	if(radius == m_startRadius)
		return;
	BaseDebugConeComponent::SetStartRadius(radius);

	NetPacket packet {};
	packet->Write<float>(radius);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetStartRadius,packet);
}
		
void SDebugConeComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet,rp);
	packet->Write<float>(m_coneAngle);
	packet->Write<float>(m_startRadius);
	packet->Write<uint32_t>(m_segmentCount);
}
luabind::object SDebugConeComponent::InitializeLuaObject(lua_State *l) {return BaseDebugConeComponent::InitializeLuaObject<SDebugConeComponentHandleWrapper>(l);}

////////////////
	
void SDebugCylinderComponent::SetLength(float length)
{
	if(length == m_length)
		return;
	BaseDebugCylinderComponent::SetLength(length);

	NetPacket packet {};
	packet->Write<float>(length);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetLength,packet);
}
		
void SDebugCylinderComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	SBaseDebugOutlineComponent::SendData(packet,rp);
	packet->Write<float>(m_length);
	packet->Write<uint32_t>(m_segmentCount);
}
luabind::object SDebugCylinderComponent::InitializeLuaObject(lua_State *l) {return BaseDebugCylinderComponent::InitializeLuaObject<SDebugCylinderComponentHandleWrapper>(l);}

////////////////
	
luabind::object SDebugPlaneComponent::InitializeLuaObject(lua_State *l) {return BaseDebugPlaneComponent::InitializeLuaObject<SDebugPlaneComponentHandleWrapper>(l);}
