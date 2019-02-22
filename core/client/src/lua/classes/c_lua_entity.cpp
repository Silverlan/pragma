#include "stdafx_client.h"
#include "pragma/lua/classes/c_lua_entity.h"

DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,Entity,BaseEntity,Entity,CLuaEntity,CLuaEntity);
CLuaEntity::CLuaEntity(luabind::object &o,const std::string &className)
	: CBaseEntity(),LuaObjectBase(o)
{
	m_class = className;
	m_luaObj = std::make_unique<luabind::object>(o);
}
void CLuaEntity::Initialize()
{
	CBaseEntity::Initialize();
	CallLuaMember("Initialize");
}
void CLuaEntity::InitializeLuaObject(lua_State*) {}
void CLuaEntity::InitializeHandle()
{
	auto *hEntity = luabind::object_cast_nothrow<CLuaEntityHandle*>(*m_luaObj).get();
	*hEntity = new PtrEntity(this);
	m_handle = hEntity;
	m_bExternalHandle = true;
}

void CLuaEntityWrapper::Initialize() {}
void CLuaEntityWrapper::default_Initialize(CLuaEntityWrapper *ent) {}

// COMPONENT TODO
#if 0
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,Entity,BaseEntity,Entity,CLuaEntity,CLuaEntity);
void CLuaEntityHandle::Reset(PtrEntity *e)
{
	EntityHandle::reset(e);
	m_bEmpty = false;
}

///////////////////////////////////////////

CLuaBaseEntity::CLuaBaseEntity(luabind::object &o,BaseEntity *ent,std::string &className)
	: LuaBaseEntity(ent,className),LuaObjectBase(o)
{}
void CLuaBaseEntity::ImplReceiveData(NetPacket &packet) {CallLuaMember<void,NetPacket>("ReceiveData",packet);}
Bool CLuaBaseEntity::ImplReceiveNetEvent(UInt32 eventId,NetPacket &packet)
{
	bool ret = false;
	if(CallLuaMember<bool,UInt32,NetPacket>("ReceiveNetEvent",&ret,eventId,packet) == CallbackReturnType::HasReturnValue)
		return ret;
	return false;
}
void CLuaBaseEntity::ImplRender(RenderMode renderMode) {CallLuaMember<void,uint32_t>("OnRender",umath::to_integral(renderMode));}
void CLuaBaseEntity::ImplPostRender(RenderMode renderMode) {CallLuaMember<void,uint32_t>("OnPostRender",umath::to_integral(renderMode));}

///////////////////////////////////////////

// COMPONENT TODO
//DEFINE_LUA_ENTITY_BASE_FUNCTIONS(CLuaEntity,CBaseEntity,);
DEFINE_LUA_ENTITY_SPECIAL_FUNCTIONS(CLuaEntity,CBaseEntity);
CLuaEntity::CLuaEntity(luabind::object &o,std::string &className)
	: CBaseEntity(),CLuaBaseEntity(o,this,className)
{
	m_luaObj = new luabind::object(o);
}

CLuaEntity::~CLuaEntity()
{}

void CLuaEntity::Render(RenderMode renderMode)
{
	// COMPONENT TODO
	//CBaseEntity::Render(renderMode);
	CLuaBaseEntity::ImplRender(renderMode);
}

void CLuaEntity::PostRender(RenderMode renderMode)
{
	// COMPONENT TODO
	//CBaseEntity::PostRender(renderMode);
	CLuaBaseEntity::ImplPostRender(renderMode);
}

void CLuaEntity::ReceiveData(NetPacket &packet)
{
	CBaseEntity::ReceiveData(packet);
	CLuaBaseEntity::ImplReceiveData(packet);
}
Bool CLuaEntity::ReceiveNetEvent(UInt32 eventId,NetPacket &packet)
{
	return (CLuaBaseEntity::ImplReceiveNetEvent(eventId,packet) || CBaseEntity::ReceiveNetEvent(eventId,packet)) ? true : false;
}

///////////////////////////////////////////

DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(CLuaEntity);
#endif
