#include "stdafx_client.h"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

extern DLLCLIENT CGame *c_game;
luabind::object CObservableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CObservableComponentHandleWrapper>(l);}
void CObservableComponent::ReceiveData(NetPacket &packet)
{
	auto b = packet->Read<bool>();
	SetFirstPersonObserverOffsetEnabled(b);
	if(b == true)
	{
		auto offset = packet->Read<Vector3>();
		SetFirstPersonObserverOffset(offset);
	}

	b = packet->Read<bool>();
	SetThirdPersonObserverOffsetEnabled(b);
	if(b == true)
	{
		auto offset = packet->Read<Vector3>();
		SetThirdPersonObserverOffset(offset);
	}
}

Bool CObservableComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetFirstPersonObserverOffset)
	{
		auto b = packet->Read<bool>();
		SetFirstPersonObserverOffsetEnabled(b);
		if(b)
		{
			auto offset = packet->Read<Vector3>();
			SetFirstPersonObserverOffset(offset);
		}
	}
	else if(eventId == m_netEvSetThirdPersonObserverOffset)
	{
		auto b = packet->Read<bool>();
		SetThirdPersonObserverOffsetEnabled(b);
		if(b)
		{
			auto offset = packet->Read<Vector3>();
			SetThirdPersonObserverOffset(offset);
		}
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}

void CObservableComponent::SetFirstPersonObserverOffset(const Vector3 &offset)
{
	BaseObservableComponent::SetFirstPersonObserverOffset(offset);
	auto *pl = c_game->GetLocalPlayer();
	if(pl == nullptr)
		return;
	auto *pObsTgt = pl->GetObserverTarget();
	if(pObsTgt == nullptr || &pObsTgt->GetEntity() != &GetEntity())
		return;
	pl->UpdateObserverOffset();
}
void CObservableComponent::SetThirdPersonObserverOffset(const Vector3 &offset)
{
	BaseObservableComponent::SetThirdPersonObserverOffset(offset);
	auto *pl = c_game->GetLocalPlayer();
	if(pl == nullptr)
		return;
	auto *pObsTgt = pl->GetObserverTarget();
	if(pObsTgt == nullptr || &pObsTgt->GetEntity() != &GetEntity())
		return;
	pl->UpdateObserverOffset();
}
