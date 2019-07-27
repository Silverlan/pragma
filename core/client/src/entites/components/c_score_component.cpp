#include "stdafx_client.h"
#include "pragma/entities/components/c_score_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

void CScoreComponent::ReceiveData(NetPacket &packet)
{
	*m_score = packet->Read<Score>();
}
luabind::object CScoreComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CScoreComponentHandleWrapper>(l);}
Bool CScoreComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetScore)
		SetScore(packet->Read<Score>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}
