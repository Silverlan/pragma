#ifndef __C_LUA_ENTITY_H__
#define __C_LUA_ENTITY_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/luaobjectbase.h>

class DLLCLIENT CLuaEntity
	: public CBaseEntity,
	public LuaObjectBase
{
public:
	CLuaEntity(luabind::object &o,const std::string &className);
	virtual void Initialize() override;
protected:
	virtual void InitializeHandle() override;
	virtual void InitializeLuaObject(lua_State *lua) override;
};
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,Entity,BaseEntity,Entity,CLuaEntity,CLuaEntity);

class DLLCLIENT CLuaEntityWrapper
	: public CLuaEntityHandle,
	public luabind::wrap_base
{
public:
	void Initialize();
	static void default_Initialize(CLuaEntityWrapper *ent);
};

#if 0
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/lua/luaapi.h>
#include "sh_lua_entity.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/entities/baseentity_handle.h>

class CLuaEntity;
DECLARE_VIRTUAL_DERIVED_CHILD_HANDLE_EXT(DLLCLIENT,Entity,BaseEntity,Entity,CLuaEntity,CLuaEntity,
	public:
		void Reset(PtrEntity *e);
);

class DLLCLIENT CLuaBaseEntity
	: public LuaBaseEntity,public LuaObjectBase
{
protected:
	void ImplReceiveData(NetPacket &packet);
	Bool ImplReceiveNetEvent(UInt32 eventId,NetPacket &packet);
	void ImplRender(RenderMode renderMode);
	void ImplPostRender(RenderMode renderMode);
	CLuaBaseEntity(luabind::object &o,BaseEntity *ent,std::string &className);
public:
	virtual void ReceiveData(NetPacket &packet)=0;
	virtual Bool ReceiveNetEvent(UInt32 eventId,NetPacket &packet)=0;
	virtual void Render(RenderMode renderMode)=0;
	virtual void PostRender(RenderMode renderMode)=0;
};

DECLARE_LUA_ENTITY_BASE(DLLCLIENT,CLuaEntity,public CBaseEntity COMMA public CLuaBaseEntity,
public:
	virtual void ReceiveData(NetPacket &packet) override;
	virtual Bool ReceiveNetEvent(UInt32 eventId,NetPacket &packet) override;
	virtual void Render(RenderMode renderMode) override;
	virtual void PostRender(RenderMode renderMode) override;
);

class DLLCLIENT CLuaBaseEntityWrapper
{
public:
	virtual void ReceiveData(NetPacket&)=0;
	static void default_ReceiveData(lua_State*,EntityHandle&,NetPacket&) {};

	virtual Bool ReceiveNetEvent(UInt32,NetPacket&)=0;
	static Bool default_ReceiveNetEvent(lua_State*,EntityHandle&,UInt32,NetPacket&) {return false;};

	virtual void OnRender(uint32_t)=0;
	static void default_OnRender(lua_State*,EntityHandle&,uint32_t) {};

	virtual void OnPostRender(uint32_t)=0;
	static void default_OnPostRender(lua_State*,EntityHandle&,uint32_t) {};
};

DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLCLIENT,CLuaEntity,public CLuaBaseEntityWrapper,
public:
	virtual void ReceiveData(NetPacket&) override {};
	virtual Bool ReceiveNetEvent(UInt32,NetPacket&) override {return false;};
	virtual void OnRender(uint32_t) override {};
	virtual void OnPostRender(uint32_t) override {};
);
#endif

#endif
