#include "stdafx_client.h"
// COMPONENT TODO
#if 0
#include "pragma/lua/c_lua_vehicle.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/physics/collision_object.hpp"

DEFINE_LUA_ENTITY_HANDLE(CLuaVehicle);

DEFINE_LUA_ENTITY_BASE_FUNCTIONS(CLuaVehicle,CBaseVehicle,{
	BaseLuaVehicle::Initialize();
});
DEFINE_LUA_ENTITY_SPECIAL_FUNCTIONS(CLuaVehicle,CBaseVehicle);
CLuaVehicle::CLuaVehicle(luabind::object &o,std::string &className)
	: CBaseVehicle(),BaseLuaVehicle(),CLuaBaseEntity(o,this,className)
{
	m_luaObj = new luabind::object(o);
}

CLuaVehicle::~CLuaVehicle()
{}

void CLuaVehicle::SetCamera(bool bFirstPerson)
{
	CBaseVehicle::SetCamera(bFirstPerson);
	CallLuaMember<void,bool>("OnCameraChanged",bFirstPerson);
}

void CLuaVehicle::Render(RenderMode renderMode)
{
	CBaseVehicle::Render(renderMode);
	CLuaBaseEntity::ImplRender(renderMode);
}

void CLuaVehicle::PostRender(RenderMode renderMode)
{
	CBaseVehicle::PostRender(renderMode);
	CLuaBaseEntity::ImplPostRender(renderMode);
}

void CLuaVehicle::ReceiveData(NetPacket &packet)
{
	CBaseVehicle::ReceiveData(packet);
	CLuaBaseEntity::ImplReceiveData(packet);
}
Bool CLuaVehicle::ReceiveNetEvent(UInt32 eventId,NetPacket &packet)
{
	return (CLuaBaseEntity::ImplReceiveNetEvent(eventId,packet) || CBaseVehicle::ReceiveNetEvent(eventId,packet)) ? true : false;
}

void CLuaVehicle::ClearDriver()
{
	BaseLuaVehicle::ClearDriver();
	CBaseVehicle::ClearDriver();
}
void CLuaVehicle::SetDriver(BaseEntity *ent)
{
	CBaseVehicle::SetDriver(ent);
	BaseLuaVehicle::SetDriver(ent);
}

///////////////////////////////////////////

DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(CLuaVehicle);

#endif
