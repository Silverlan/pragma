#include "stdafx_server.h"
#include "pragma/entities/components/s_color_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>

using namespace pragma;

void SColorComponent::Initialize()
{
	BaseColorComponent::Initialize();
	FlagCallbackForRemoval(m_color->AddCallback([this](std::reference_wrapper<const Color> oldVal,std::reference_wrapper<const Color> val) {
		NetPacket p {};
		p->Write<Color>(val);
		static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetColor,p);
	}),CallbackType::Component,this);
}

luabind::object SColorComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SColorComponentHandleWrapper>(l);}

void SColorComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<Color>(*m_color);
}
