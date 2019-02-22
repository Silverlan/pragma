#include "stdafx_client.h"
// COMPONENT TODO
#if 0
#include "pragma/lua/c_lua_npc.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/physics/physcollisionobject.h"

DEFINE_LUA_ENTITY_HANDLE(CLuaNPC);

DEFINE_LUA_ENTITY_BASE_FUNCTIONS(CLuaNPC,CBaseNPC,);
DEFINE_LUA_ENTITY_SPECIAL_FUNCTIONS(CLuaNPC,CBaseNPC);
CLuaNPC::CLuaNPC(luabind::object &o,std::string &className)
	: CBaseNPC(),CLuaBaseEntity(o,this,className),BaseSharedLuaNPC()
{
	m_luaObj = new luabind::object(o);
}

CLuaNPC::~CLuaNPC()
{}

Vector2 CLuaNPC::CalcMovementSpeed() const
{
	auto speed = CBaseNPC::CalcMovementSpeed();
	BaseSharedLuaNPC::CalcMovementSpeed(*const_cast<CLuaNPC*>(this),speed);
	return speed;
}

void CLuaNPC::Render(RenderMode renderMode)
{
	CBaseNPC::Render(renderMode);
	CLuaBaseEntity::ImplRender(renderMode);
}

void CLuaNPC::PostRender(RenderMode renderMode)
{
	CBaseNPC::PostRender(renderMode);
	CLuaBaseEntity::ImplPostRender(renderMode);
}

void CLuaNPC::ReceiveData(NetPacket &packet)
{
	CBaseNPC::ReceiveData(packet);
	CLuaBaseEntity::ImplReceiveData(packet);
}
Bool CLuaNPC::ReceiveNetEvent(UInt32 eventId,NetPacket &packet)
{
	return (CLuaBaseEntity::ImplReceiveNetEvent(eventId,packet) || CBaseNPC::ReceiveNetEvent(eventId,packet)) ? true : false;
}
void CLuaNPC::PlayFootStepSound(BaseCharacter::FootType foot,const SurfaceMaterial &surfMat,float scale)
{
	auto ret = false;
	if(
		CallLuaMember<bool,uint32_t,SurfaceMaterial*,float>("PlayFootStepSound",&ret,umath::to_integral(foot),const_cast<SurfaceMaterial*>(&surfMat),scale) == CallbackReturnType::HasReturnValue && 
		ret == true
	)
		return;
	CBaseNPC::PlayFootStepSound(foot,surfMat,scale);
}

///////////////////////////////////////////

DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(CLuaNPC);

#endif
