#include "stdafx_client.h"
#include "pragma/lua/classes/c_lentity.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/lentity.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <networkmanager/nwm_packet.h>
#include <networkmanager/interface/nwm_manager.hpp>

void Lua::Entity::Client::register_class(luabind::class_<EntityHandle> &classDef)
{
	::Lua::Entity::register_class(classDef);
	classDef.def("IsClientsideOnly",&IsClientsideOnly);
	classDef.def("GetClientIndex",&GetClientIndex);
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t protocol,unsigned int,const NetPacket&)>(&SendNetEvent));
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t protocol,unsigned int)>(&SendNetEvent));
}

void Lua::Entity::Client::IsClientsideOnly(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,static_cast<CBaseEntity*>(hEnt.get())->IsClientsideOnly());
}

void Lua::Entity::Client::GetClientIndex(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushInt(l,static_cast<CBaseEntity*>(hEnt.get())->GetClientIndex());
}
void Lua::Entity::Client::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,const NetPacket &packet)
{
	LUA_CHECK_ENTITY(l,hEnt);
	switch(static_cast<nwm::Protocol>(protocol))
	{
		case nwm::Protocol::TCP:
			static_cast<CBaseEntity*>(hEnt.get())->SendNetEventTCP(eventId,const_cast<NetPacket&>(packet));
			break;
		case nwm::Protocol::UDP:
			static_cast<CBaseEntity*>(hEnt.get())->SendNetEventUDP(eventId,const_cast<NetPacket&>(packet));
			break;
	}
}
void Lua::Entity::Client::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId) {SendNetEvent(l,hEnt,protocol,eventId,{});}
