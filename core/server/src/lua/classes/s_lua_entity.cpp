#include "stdafx_server.h"
#include "pragma/lua/classes/s_lua_entity.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/networking/wvserverclient.h"
#include <pragma/entities/components/map_component.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/networking/nwm_util.h>

extern DLLSERVER SGame *s_game;
extern DLLSERVER ServerState *server;

DEFINE_DERIVED_CHILD_HANDLE(DLLSERVER,Entity,BaseEntity,Entity,SLuaEntity,SLuaEntity);
SLuaEntity::SLuaEntity(luabind::object &o,const std::string &className)
	: SBaseEntity(),LuaObjectBase(o)
{
	m_class = className;
	m_luaObj = std::make_unique<luabind::object>(o);
}
void SLuaEntity::Initialize()
{
	SBaseEntity::Initialize();
	CallLuaMember("Initialize");
}
bool SLuaEntity::IsScripted() const {return true;}
void SLuaEntity::InitializeLuaObject(lua_State*) {}
void SLuaEntity::InitializeHandle()
{
	auto *hEntity = luabind::object_cast_nothrow<SLuaEntityHandle*>(*m_luaObj).get();
	*hEntity = new PtrEntity(this);
	m_handle = hEntity;
	m_bExternalHandle = true;
}

void SLuaEntity::DoSpawn()
{
	SBaseEntity::DoSpawn();
	if(IsShared())
	{
		nwm::RecipientFilter rp;
		EntityIterator entIt {*s_game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::Player};
		for(auto *entPl : entIt)
		{
			if(entPl != nullptr)
			{
				auto &pPlComponent = static_cast<pragma::SPlayerComponent&>(*entPl->GetPlayerComponent());
				rp.Add(pPlComponent.GetClientSession());
			}
		}
		NetPacket p;
		p->WriteString(GetClass());
		p->Write<unsigned int>(GetIndex());
		auto pMapComponent = GetComponent<pragma::MapComponent>();
		p->Write<unsigned int>(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
		SendData(p,rp);
		server->SendPacketTCP("ent_create_lua",p,rp);
	}
}
void SLuaEntity::Remove()
{
	if(IsSpawned())
	{
		// TODO: Do we need this? (If so, why?)
		NetPacket p;
		nwm::write_entity(p,this);
		server->BroadcastTCP("ent_remove",p);
	}
	SBaseEntity::Remove();
}

///////////

void SLuaEntityWrapper::Initialize() {}
void SLuaEntityWrapper::default_Initialize(SLuaEntityWrapper *ent) {}

#if 0
#include "pragma/lua/classes/s_lua_entity.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/physics/physcollisionobject.h"

DEFINE_DERIVED_CHILD_HANDLE(DLLSERVER,Entity,BaseEntity,Entity,SLuaEntity,SLuaEntity);
void SLuaEntityHandle::Reset(PtrEntity *e)
{
	EntityHandle::reset(e);
	m_bEmpty = false;
}

///////////////////////////////////////////

SLuaBaseEntity::SLuaBaseEntity(luabind::object &o,BaseEntity *ent,std::string &className)
	: LuaBaseEntity(ent,className),LuaObjectBase(o)
{}
// COMPONENT TODO
/*Bool SLuaBaseEntity::ImplReceiveNetEvent(Player *pl,UInt32 eventId,NetPacket &packet)
{
	bool ret = false;
	if(CallLuaMember<bool,luabind::object,UInt32,NetPacket>("ReceiveNetEvent",&ret,*pl->GetLuaObject(),eventId,packet) == CallbackReturnType::HasReturnValue)
		return ret;
	return false;
}*/
void SLuaBaseEntity::ImplSendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	CallLuaMember<void,NetPacket,nwm::RecipientFilter>("SendData",packet,rp);
}

///////////////////////////////////////////

// COMPONENT TODO
//DEFINE_LUA_ENTITY_BASE_FUNCTIONS(SLuaEntity,Entity,);
// COMPONENT TODO
//DEFINE_LUA_S_ENTITY_SPECIAL_FUNCTIONS(SLuaEntity,Entity);

// COMPONENT TODO (Remove these)
void SLuaEntity::DoSpawn()
{
}
void SLuaEntity::OnPostSpawn()
{
}
void SLuaEntity::Remove()
{
}
//

SLuaEntity::SLuaEntity(luabind::object &o,std::string &className)
	: Entity(),SLuaBaseEntity(o,this,className)
{
	m_luaObj = new luabind::object(o);
}

SLuaEntity::~SLuaEntity()
{}

void SLuaEntity::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	Entity::SendData(packet,rp);
	SLuaBaseEntity::ImplSendData(packet,rp);
}

// COMPONENT TODO
Bool SLuaEntity::ReceiveNetEvent(pragma::BasePlayerComponent *pl,UInt32 eventId,NetPacket &packet)
{
	return false;//(SLuaBaseEntity::ImplReceiveNetEvent(pl,eventId,packet) || Entity::ReceiveNetEvent(pl,eventId,packet)) ? true : false;
}

///////////////////////////////////////////

//DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(SLuaEntity);
#endif
