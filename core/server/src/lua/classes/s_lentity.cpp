#include "stdafx_server.h"
#include "pragma/lua/classes/s_lentity.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/lua/classes/lentity.h"
#include "luasystem.h"
#include <pragma/audio/alsound_type.h>
#include <servermanager/interface/sv_nwm_manager.hpp>

void Lua::Entity::Server::register_class(luabind::class_<EntityHandle> &classDef)
{
	::Lua::Entity::register_class(classDef);
	classDef.def("IsShared",&IsShared);
	classDef.def("SetShared",&SetShared);
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,unsigned int,::NetPacket,nwm::RecipientFilter&)>(&SendNetEvent));
	classDef.def("BroadcastNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,unsigned int,::NetPacket)>(&SendNetEvent));
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,unsigned int,nwm::RecipientFilter&)>(&SendNetEvent));
	classDef.def("BroadcastNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,unsigned int)>(&SendNetEvent));
	classDef.def("IsSynchronized",&IsSynchronized);
	classDef.def("SetSynchronized",&SetSynchronized);
	classDef.def("SetSnapshotDirty",&SetSnapshotDirty);
}

void Lua::Entity::Server::IsShared(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,hEnt.get<SBaseEntity>()->IsShared());
}
void Lua::Entity::Server::SetShared(lua_State *l,EntityHandle &hEnt,bool b)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt.get<SBaseEntity>()->SetShared(b);
}

void Lua::Entity::Server::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,::NetPacket packet,nwm::RecipientFilter &rp)
{
	LUA_CHECK_ENTITY(l,hEnt);
	switch(static_cast<nwm::Protocol>(protocol))
	{
		case nwm::Protocol::TCP:
			static_cast<SBaseEntity*>(hEnt.get())->SendNetEventTCP(eventId,packet,rp);
			break;
		case nwm::Protocol::UDP:
			static_cast<SBaseEntity*>(hEnt.get())->SendNetEventUDP(eventId,packet,rp);
			break;
	}
}
void Lua::Entity::Server::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,NetPacket packet)
{
	LUA_CHECK_ENTITY(l,hEnt);
	switch(static_cast<nwm::Protocol>(protocol))
	{
		case nwm::Protocol::TCP:
			static_cast<SBaseEntity*>(hEnt.get())->SendNetEventTCP(eventId,packet);
			break;
		case nwm::Protocol::UDP:
			static_cast<SBaseEntity*>(hEnt.get())->SendNetEventUDP(eventId,packet);
			break;
	}
}
void Lua::Entity::Server::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,nwm::RecipientFilter &rp) {SendNetEvent(l,hEnt,protocol,eventId,{},rp);}
void Lua::Entity::Server::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId) {SendNetEvent(l,hEnt,protocol,eventId,{});}
void Lua::Entity::Server::IsSynchronized(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,hEnt.get<SBaseEntity>()->IsSynchronized());
}
void Lua::Entity::Server::SetSynchronized(lua_State *l,EntityHandle &hEnt,Bool b)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt.get<SBaseEntity>()->SetSynchronized(b);
}
void Lua::Entity::Server::SetSnapshotDirty(lua_State *l,EntityHandle &hEnt,bool b)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt->MarkForSnapshot(b);
}
