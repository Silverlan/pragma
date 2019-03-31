#include "stdafx_server.h"
#include "pragma/entities/components/s_radius_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>

using namespace pragma;

void SRadiusComponent::Initialize()
{
	BaseRadiusComponent::Initialize();
	FlagCallbackForRemoval(m_radius->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> val) {
		NetPacket p {};
		p->Write<float>(val.get());
		static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetRadius,p);
	}),CallbackType::Component,this);
}
luabind::object SRadiusComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SRadiusComponentHandleWrapper>(l);}

void SRadiusComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<float>(*m_radius);
}
