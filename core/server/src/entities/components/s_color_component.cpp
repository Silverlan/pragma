#include "stdafx_server.h"
#include "pragma/entities/components/s_color_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

using namespace pragma;

void SColorComponent::Initialize()
{
	BaseColorComponent::Initialize();
	FlagCallbackForRemoval(m_color->AddCallback([this](std::reference_wrapper<const Color> oldVal,std::reference_wrapper<const Color> val) {
		NetPacket p {};
		p->Write<Color>(val);
		static_cast<SBaseEntity&>(GetEntity()).SendNetEvent(m_netEvSetColor,p,pragma::networking::Protocol::SlowReliable);
	}),CallbackType::Component,this);
}

luabind::object SColorComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SColorComponentHandleWrapper>(l);}

void SColorComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->Write<Color>(*m_color);
}
