#ifndef __C_LUA_WEAPON_H__
#define __C_LUA_WEAPON_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseweapon.h"
#include "pragma/lua/classes/c_lua_entity.h"
/*
class CLuaWeapon;
DECLARE_LUA_ENTITY_HANDLE(DLLCLIENT,CLuaWeapon,WeaponHandle,CLuaEntity);

DECLARE_LUA_ENTITY_BASE(DLLCLIENT,CLuaWeapon,public CBaseWeapon COMMA public CLuaBaseEntity,
	DECLARE_LUA_WEAPON_BASE_FUNCTIONS
public:
	virtual bool HandleViewModelAnimationEvent(CViewModel *vm,const AnimationEvent &ev) override;
	virtual Activity TranslateViewActivity(Activity act) override;

	virtual void ReceiveData(NetPacket &packet) override;
	virtual Bool ReceiveNetEvent(UInt32 eventId,NetPacket &packet) override;
	virtual void Render(RenderMode renderMode) override;
	virtual void PostRender(RenderMode renderMode) override;
);

DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLCLIENT,CLuaWeapon,LuaWeaponWrapper COMMA public CLuaBaseEntityWrapper,
public:
	virtual void ReceiveData(NetPacket&) override {};
	virtual Bool ReceiveNetEvent(UInt32,NetPacket&) override {return false;};
	virtual void OnRender(uint32_t) override {};
	virtual void OnPostRender(uint32_t) override {};

	void HandleViewModelAnimationEvent(EntityHandle &hVm,uint32_t evId,luabind::object tArgs);
	static void default_HandleViewModelAnimationEvent(lua_State *l,EntityHandle &hEnt,EntityHandle &hVm,uint32_t evId,luabind::object tArgs);

	unsigned short TranslateViewActivity(unsigned short act);
	static unsigned short default_TranslateViewActivity(lua_State *l,EntityHandle &hEnt,unsigned short act);
);
*/
#endif