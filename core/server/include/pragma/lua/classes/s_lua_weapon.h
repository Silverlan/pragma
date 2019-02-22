#ifndef __S_LUA_WEAPON_H__
#define __S_LUA_WEAPON_H__

#include "pragma/serverdefinitions.h"
#include "pragma/lua/classes/s_lua_entity.h"
/*
class SLuaWeapon;
DECLARE_LUA_ENTITY_HANDLE(DLLSERVER,SLuaWeapon,WeaponHandle,SLuaEntity);

DECLARE_LUA_ENTITY_BASE(DLLSERVER,SLuaWeapon,public SBaseWeapon COMMA public SLuaBaseEntity,
public:
	DECLARE_LUA_WEAPON_BASE_FUNCTIONS

	virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
	virtual Bool ReceiveNetEvent(Player *pl,UInt32 eventId,NetPacket &packet) override;
	virtual void OnPickedUp(BaseEntity *ent) override;
);

DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLSERVER,SLuaWeapon,LuaWeaponWrapper COMMA public SLuaBaseEntityWrapper,
public:
	void Initialize();
	static void default_Initialize(lua_State *l,SLuaWeaponHandle &ptr);

	void Drop();
	static void default_Drop(lua_State *l,SLuaWeaponHandle &hEnt);

	void OnPickedUp(EntityHandle &hActivator);
	static void default_OnPickedUp(lua_State *l,SLuaWeaponHandle &hEnt,EntityHandle &hActivator);

	virtual void SendData(NetPacket&,nwm::RecipientFilter&) override {};
	virtual Bool ReceiveNetEvent(PlayerHandle&,UInt32,NetPacket&) override {return false;};
);
*/
//.def("Drop",&LuaWeaponWrapper::Drop<classname>,&LuaWeaponWrapper::default_Drop) \

#endif