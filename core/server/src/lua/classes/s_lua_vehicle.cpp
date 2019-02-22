#include "stdafx_server.h"
#if 0
#include "pragma/lua/classes/s_lua_vehicle.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/physics/physcollisionobject.h"

DEFINE_LUA_ENTITY_HANDLE(SLuaVehicle);

DEFINE_LUA_ENTITY_BASE_FUNCTIONS(SLuaVehicle,SBaseVehicle,{
	BaseLuaVehicle::Initialize();
});
DEFINE_LUA_S_ENTITY_SPECIAL_FUNCTIONS(SLuaVehicle,SBaseVehicle);
SLuaVehicle::SLuaVehicle(luabind::object &o,std::string &className)
	: SBaseVehicle(),SLuaBaseEntity(o,this,className)
{
	m_luaObj = new luabind::object(o);
}

SLuaVehicle::~SLuaVehicle()
{}

void SLuaVehicle::ClearDriver()
{
	BaseLuaVehicle::ClearDriver();
	SBaseVehicle::ClearDriver();
}
void SLuaVehicle::SetDriver(BaseEntity *ent)
{
	SBaseVehicle::SetDriver(ent);
	BaseLuaVehicle::SetDriver(ent);
}

void SLuaVehicle::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	SBaseVehicle::SendData(packet,rp);
	SLuaBaseEntity::ImplSendData(packet,rp);
}

Bool SLuaVehicle::ReceiveNetEvent(Player *pl,UInt32 eventId,NetPacket &packet)
{
	return (SLuaBaseEntity::ImplReceiveNetEvent(pl,eventId,packet) || SBaseVehicle::ReceiveNetEvent(pl,eventId,packet)) ? true : false;
}

///////////////////////////////////////////

DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(SLuaVehicle);
#endif
