#include "stdafx_client.h"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <stack>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

luabind::object CTransformComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CTransformComponentHandleWrapper>(l);}
void CTransformComponent::ReceiveData(NetPacket &packet)
{
	Vector3 pos = nwm::read_vector(packet);
	auto rot = nwm::read_quat(packet);
	SetPosition(pos);
	SetOrientation(rot);
	SetEyeOffset(packet->Read<Vector3>());

	auto scale = packet->Read<Vector3>();
	SetScale(scale);
}

Bool CTransformComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetScale)
		SetScale(packet->Read<Vector3>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}
