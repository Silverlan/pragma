#ifndef __S_LUA_ENTITY_H__
#define __S_LUA_ENTITY_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/luaobjectbase.h>

class DLLSERVER SLuaEntity
	: public SBaseEntity,
	public LuaObjectBase
{
public:
	SLuaEntity(luabind::object &o,const std::string &className);
	virtual void Initialize() override;

	virtual bool IsScripted() const override;
	virtual void DoSpawn() override;
	virtual void Remove() override;
protected:
	virtual void InitializeHandle() override;
	virtual void InitializeLuaObject(lua_State *lua) override;
};
DECLARE_DERIVED_CHILD_HANDLE(DLLSERVER,Entity,BaseEntity,Entity,SLuaEntity,SLuaEntity);

class DLLSERVER SLuaEntityWrapper
	: public SLuaEntityHandle,
	public luabind::wrap_base
{
public:
	void Initialize();
	static void default_Initialize(SLuaEntityWrapper *ent);
};

// Component TODO
#if 0
#include "pragma/entities/s_baseentity.h"
#include <pragma/lua/luaapi.h>
#include "sh_lua_entity.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/entities/baseentity_handle.h>
#include "sv_nwm_recipientfilter.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/entities/player.h"

#define DEFINE_LUA_S_ENTITY_SPECIAL_FUNCTIONS(classname,baseclass) \
	extern DLLSERVER ServerState *server; \
	void classname::DoSpawn() \
	{ \
		baseclass::DoSpawn(); \
		LOnSpawn(); \
		if(IsShared()) \
		{ \
			nwm::RecipientFilter rp; \
			std::vector<Player*> *players; \
			Player::GetAll(&players); \
			for(unsigned int i=0;i<players->size();i++) \
			{ \
				Player *pl = (*players)[i]; \
				if(pl != nullptr && pl->IsSpawned()) \
					rp.Add(pl->GetClientSession()); \
			} \
			NetPacket p; \
			p->WriteString(GetClass()); \
			p->Write<unsigned int>(GetIndex()); \
			p->Write<unsigned int>(GetMapIndex()); \
			SendData(p,rp); \
			server->SendPacketTCP("ent_create_lua",p,rp); \
		} \
	} \
	void classname::OnPostSpawn() \
	{ \
		baseclass::OnPostSpawn(); \
		LOnPostSpawn(); \
	} \
	void classname::Remove() \
	{ \
		if(IsSpawned()) \
		{ \
			NetPacket p; \
			nwm::write_entity(p,this); \
			server->BroadcastTCP("ent_remove",p); \
		} \
		OnRemove(); \
		baseclass::Remove(); \
	}

class SLuaEntity;
DECLARE_VIRTUAL_DERIVED_CHILD_HANDLE_EXT(DLLSERVER,Entity,BaseEntity,Entity,SLuaEntity,SLuaEntity,
	public:
		void Reset(PtrEntity *e);
);

class DLLSERVER SLuaBaseEntity
	: public LuaBaseEntity,public LuaObjectBase
{
protected:
	Bool ImplReceiveNetEvent(pragma::BasePlayerComponent *pl,UInt32 eventId,NetPacket &packet);
	void ImplSendData(NetPacket &packet,nwm::RecipientFilter &rp);
	SLuaBaseEntity(luabind::object &o,BaseEntity *ent,std::string &className);
public:
	virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent *pl,UInt32 eventId,NetPacket &packet)=0;
	virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp)=0;
};

DECLARE_LUA_ENTITY_BASE(DLLSERVER,SLuaEntity,public Entity COMMA public SLuaBaseEntity,
public:
	virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
	virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent *pl,UInt32 eventId,NetPacket &packet) override;
);

class DLLSERVER SLuaBaseEntityWrapper
{
public:
	virtual void SendData(NetPacket &p,nwm::RecipientFilter &rp)=0;
	static void default_SendData(lua_State*,NetPacket&,nwm::RecipientFilter&) {};

	// COMPONENT TODO
	//virtual Bool ReceiveNetEvent(PlayerHandle &hPl,UInt32 eventId,NetPacket &packet)=0;
	//static Bool default_ReceiveNetEvent(lua_State*,EntityHandle&,PlayerHandle&,UInt32,NetPacket&) {return false;};
};

DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLSERVER,SLuaEntity,public SLuaBaseEntityWrapper,
public:
	// COMPONENT TODO
	//virtual void SendData(NetPacket&,nwm::RecipientFilter&) override {}
	//virtual Bool ReceiveNetEvent(PlayerHandle&,UInt32,NetPacket&) override {return false;}
);
#endif

#endif