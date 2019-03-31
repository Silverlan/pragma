#include "stdafx_server.h"
#include "pragma/entities/components/s_observable_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>

using namespace pragma;

void SObservableComponent::SetFirstPersonObserverOffset(const Vector3 &offset)
{
	BaseObservableComponent::SetFirstPersonObserverOffset(offset);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	if(IsFirstPersonModeEnabled() == false)
		p->Write<bool>(false);
	else
	{
		p->Write<bool>(true);
		p->Write<Vector3>(offset);
	}
	ent.SendNetEventTCP(m_netEvSetFirstPersonObserverOffset,p);
}
luabind::object SObservableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SObservableComponentHandleWrapper>(l);}
void SObservableComponent::SetThirdPersonObserverOffset(const Vector3 &offset)
{
	BaseObservableComponent::SetThirdPersonObserverOffset(offset);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	if(IsThirdPersonModeEnabled() == false)
		p->Write<bool>(false);
	else
	{
		p->Write<bool>(true);
		p->Write<Vector3>(offset);
	}
	ent.SendNetEventTCP(m_netEvSetThirdPersonObserverOffset,p);
}
void SObservableComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	if(*m_bFirstPersonEnabled == false)
		packet->Write<bool>(false);
	else
	{
		packet->Write<bool>(true);
		packet->Write<Vector3>(*m_firstPersonObserverOffset);
	}

	if(*m_bThirdPersonEnabled == false)
		packet->Write<bool>(false);
	else
	{
		packet->Write<bool>(true);
		packet->Write<Vector3>(*m_thirdPersonObserverOffset);
	}
}
