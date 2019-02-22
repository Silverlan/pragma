#include "stdafx_client.h"
#include "pragma/lua/c_lua_weapon.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/entities/c_viewmodel.h"

/*DEFINE_LUA_ENTITY_HANDLE(CLuaWeapon);

DEFINE_LUA_ENTITY_BASE_FUNCTIONS(CLuaWeapon,CBaseWeapon,);
DEFINE_LUA_ENTITY_SPECIAL_FUNCTIONS(CLuaWeapon,CBaseWeapon);
DEFINE_LUA_WEAPON_BASE_FUNCTIONS(CLuaWeapon,CBaseWeapon);
CLuaWeapon::CLuaWeapon(luabind::object &o,std::string &className)
	: CBaseWeapon(),CLuaBaseEntity(o,this,className)
{
	m_luaObj = new luabind::object(o);
}

CLuaWeapon::~CLuaWeapon()
{}

void CLuaWeapon::Render(RenderMode renderMode)
{
	CBaseWeapon::Render(renderMode);
	CLuaBaseEntity::ImplRender(renderMode);
}

void CLuaWeapon::PostRender(RenderMode renderMode)
{
	CBaseWeapon::PostRender(renderMode);
	CLuaBaseEntity::ImplPostRender(renderMode);
}

void CLuaWeapon::ReceiveData(NetPacket &packet)
{
	CBaseWeapon::ReceiveData(packet);
	CLuaBaseEntity::ImplReceiveData(packet);
}
Bool CLuaWeapon::ReceiveNetEvent(UInt32 eventId,NetPacket &packet)
{
	return (CLuaBaseEntity::ImplReceiveNetEvent(eventId,packet) || CBaseWeapon::ReceiveNetEvent(eventId,packet)) ? true : false;
}

bool CLuaWeapon::HandleViewModelAnimationEvent(CViewModel *vm,const AnimationEvent &ev)
{
	auto *nw = GetNetworkState();
	auto *game = nw->GetGameState();
	auto *l = game->GetLuaState();
	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto it=ev.arguments.begin();it!=ev.arguments.end();++it)
		t[idx] = *it;
	auto r = CallLuaMember<bool,EntityHandle,unsigned int,luabind::object>("HandleViewModelAnimationEvent",vm->GetHandle(),umath::to_integral(ev.eventID),t);
	if(r == true)
		return r;
	return CBaseWeapon::HandleViewModelAnimationEvent(vm,ev);
}

Activity CLuaWeapon::TranslateViewActivity(Activity act)
{
	auto ract = umath::to_integral(act);
	if(CallLuaMember<unsigned short,unsigned short>("TranslateViewActivity",&ract,ract) == CallbackReturnType::HasReturnValue)
		return static_cast<Activity>(ract);
	return CBaseWeapon::TranslateViewActivity(static_cast<Activity>(ract));
}

///////////////////////////////////////////

DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(CLuaWeapon);

void CLuaWeaponWrapper::HandleViewModelAnimationEvent(EntityHandle&,uint32_t,luabind::object) {}
void CLuaWeaponWrapper::default_HandleViewModelAnimationEvent(lua_State*,EntityHandle&,EntityHandle&,uint32_t,luabind::object) {}

unsigned short CLuaWeaponWrapper::TranslateViewActivity(unsigned short act) {return act;}
unsigned short CLuaWeaponWrapper::default_TranslateViewActivity(lua_State*,EntityHandle&,unsigned short act) {return act;}
*/