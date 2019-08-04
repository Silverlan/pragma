#include "stdafx_server.h"
#include "pragma/entities/components/s_score_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

using namespace pragma;

void SScoreComponent::Initialize()
{
	BaseScoreComponent::Initialize();
	FlagCallbackForRemoval(m_score->AddCallback([this](std::reference_wrapper<const Score> oldVal,std::reference_wrapper<const Score> val) {
		NetPacket p {};
		p->Write<Score>(val);
		static_cast<SBaseEntity&>(GetEntity()).SendNetEvent(m_netEvSetScore,p,pragma::networking::Protocol::SlowReliable);
	}),CallbackType::Component,this);
}

luabind::object SScoreComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SScoreComponentHandleWrapper>(l);}

void SScoreComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->Write<Score>(*m_score);
}
