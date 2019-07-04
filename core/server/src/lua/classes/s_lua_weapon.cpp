#include "stdafx_server.h"
#include "pragma/lua/classes/s_lua_weapon.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/physics/collision_object.hpp"
/*
DEFINE_LUA_ENTITY_HANDLE(SLuaWeapon);

DEFINE_LUA_ENTITY_BASE_FUNCTIONS(SLuaWeapon,SBaseWeapon,);
DEFINE_LUA_S_ENTITY_SPECIAL_FUNCTIONS(SLuaWeapon,SBaseWeapon);
DEFINE_LUA_WEAPON_BASE_FUNCTIONS(SLuaWeapon,SBaseWeapon);
SLuaWeapon::SLuaWeapon(luabind::object &o,std::string &className)
	: SBaseWeapon(),SLuaBaseEntity(o,this,className)
{
	m_luaObj = new luabind::object(o);
}

SLuaWeapon::~SLuaWeapon()
{}

void SLuaWeapon::OnPickedUp(BaseEntity *ent)
{
	SBaseWeapon::OnPickedUp(ent);
	CallLuaMember<void,luabind::object>("OnPickedUp",(ent != nullptr) ? *ent->GetLuaObject() : luabind::object{});
}

void SLuaWeapon::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	SBaseWeapon::SendData(packet,rp);
	SLuaBaseEntity::ImplSendData(packet,rp);
}

Bool SLuaWeapon::ReceiveNetEvent(Player *pl,UInt32 eventId,NetPacket &packet)
{
	return (SLuaBaseEntity::ImplReceiveNetEvent(pl,eventId,packet) || SBaseWeapon::ReceiveNetEvent(pl,eventId,packet)) ? true : false;
}

///////////////////////////////////////////

DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(SLuaWeapon);

void SLuaWeaponWrapper::Initialize() {}
void SLuaWeaponWrapper::default_Initialize(lua_State*,SLuaWeaponHandle&) {}

void SLuaWeaponWrapper::Drop() {}
void SLuaWeaponWrapper::default_Drop(lua_State *l,SLuaWeaponHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWeapon*>(hEnt.get())->Drop();
}

void SLuaWeaponWrapper::OnPickedUp(EntityHandle &hActivator) {}
void SLuaWeaponWrapper::default_OnPickedUp(lua_State *l,SLuaWeaponHandle &hEnt,EntityHandle &hActivator)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWeapon*>(hEnt.get())->OnPickedUp(hActivator.get());
}
*/