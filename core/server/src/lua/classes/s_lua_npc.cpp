#include "stdafx_server.h"
#if 0
#include "pragma/lua/classes/s_lua_npc.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/physics/physcollisionobject.h"

DEFINE_LUA_ENTITY_HANDLE(SLuaNPC);

DEFINE_LUA_ENTITY_BASE_FUNCTIONS(SLuaNPC,SBaseNPC,);
DEFINE_LUA_S_ENTITY_SPECIAL_FUNCTIONS(SLuaNPC,SBaseNPC);
SLuaNPC::SLuaNPC(luabind::object &o,std::string &className)
	: SBaseNPC(),SLuaBaseEntity(o,this,className),BaseSharedLuaNPC()
{
	m_luaObj = new luabind::object(o);
}

SLuaNPC::~SLuaNPC()
{}

Vector2 SLuaNPC::CalcMovementSpeed() const
{
	auto speed = SBaseNPC::CalcMovementSpeed();
	BaseSharedLuaNPC::CalcMovementSpeed(*const_cast<SLuaNPC*>(this),speed);
	return speed;
}

void SLuaNPC::SelectSchedule()
{
	SBaseNPC::SelectSchedule();
	CallLuaMember<void>("SelectSchedule");
}

void SLuaNPC::SelectControllerSchedule()
{
	SBaseNPC::SelectControllerSchedule();
	CallLuaMember<void>("SelectControllerSchedule");
}

void SLuaNPC::OnPrimaryTargetChanged(const ai::Memory::Fragment *memFragment)
{
	SBaseNPC::OnPrimaryTargetChanged(memFragment);
	CallLuaMember<void,ai::Memory::Fragment*>("OnPrimaryTargetChanged",const_cast<ai::Memory::Fragment*>(memFragment));
}

void SLuaNPC::OnNPCStateChanged(NPCSTATE oldState,NPCSTATE newState)
{
	SBaseNPC::OnNPCStateChanged(oldState,newState);
	CallLuaMember<void,uint32_t,uint32_t>("OnNPCStateChanged",umath::to_integral(oldState),umath::to_integral(newState));
}

void SLuaNPC::OnTargetVisibilityLost(const ai::Memory::Fragment &memFragment)
{
	SBaseNPC::OnTargetVisibilityLost(memFragment);
	CallLuaMember<void,ai::Memory::Fragment*>("OnTargetVisibilityLost",const_cast<ai::Memory::Fragment*>(&memFragment));
}
void SLuaNPC::OnTargetVisibilityReacquired(const ai::Memory::Fragment &memFragment)
{
	SBaseNPC::OnTargetVisibilityReacquired(memFragment);
	CallLuaMember<void,ai::Memory::Fragment*>("OnTargetVisibilityReacquired",const_cast<ai::Memory::Fragment*>(&memFragment));
}
void SLuaNPC::OnMemoryGained(const ai::Memory::Fragment &memFragment)
{
	SBaseNPC::OnMemoryGained(memFragment);
	CallLuaMember<void,ai::Memory::Fragment*>("OnMemoryGained",const_cast<ai::Memory::Fragment*>(&memFragment));
}
void SLuaNPC::OnMemoryLost(const ai::Memory::Fragment &memFragment)
{
	SBaseNPC::OnMemoryLost(memFragment);
	CallLuaMember<void,ai::Memory::Fragment*>("OnMemoryLost",const_cast<ai::Memory::Fragment*>(&memFragment));
}

void SLuaNPC::OnTargetAcquired(BaseEntity *ent,float dist,bool bFirst)
{
	SBaseNPC::OnTargetAcquired(ent,dist,bFirst);
	CallLuaMember<void,luabind::object,float,bool>("OnTargetAcquired",*ent->GetLuaObject(),dist,bFirst);
}

void SLuaNPC::OnScheduleComplete()
{
	SBaseNPC::OnScheduleComplete();
	CallLuaMember<void>("OnScheduleComplete");
}

bool SLuaNPC::OnSuspiciousSoundHeared(std::shared_ptr<ALSound> &snd)
{
	auto ret = false;
	if(CallLuaMember<bool,std::shared_ptr<ALSound>>("OnSuspiciousSoundHeared",&ret,snd) == CallbackReturnType::HasReturnValue && ret == true)
		return ret;
	return SBaseNPC::OnSuspiciousSoundHeared(snd);
}

void SLuaNPC::OnControllerActionInput(Action action,bool b)
{
	CallLuaMember<void,std::underlying_type_t<Action>,bool>("OnControllerActionInput",umath::to_integral(action),b);
}
void SLuaNPC::OnStartControl(Player *pl)
{
	CallLuaMember<void,luabind::object>("OnStartControl",*pl->GetLuaObject());
}
void SLuaNPC::OnEndControl()
{
	CallLuaMember<void>("OnEndControl");
}
void SLuaNPC::OnDeath(DamageInfo *damageInfo)
{
	auto ret = false;
	if(CallLuaMember<bool,DamageInfo*>("OnDeath",&ret,damageInfo) == CallbackReturnType::HasReturnValue && ret == true)
		return;
	SBaseNPC::OnDeath(damageInfo);
}

void SLuaNPC::PlayFootStepSound(BaseCharacter::FootType foot,const SurfaceMaterial &surfMat,float scale)
{
	auto ret = false;
	if(CallLuaMember<bool,uint32_t,SurfaceMaterial*,float>("PlayFootStepSound",&ret,umath::to_integral(foot),const_cast<SurfaceMaterial*>(&surfMat),scale) == CallbackReturnType::HasReturnValue && ret == true)
		return;
	SBaseNPC::PlayFootStepSound(foot,surfMat,scale);
}

void SLuaNPC::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	SBaseNPC::SendData(packet,rp);
	SLuaBaseEntity::ImplSendData(packet,rp);
}

Bool SLuaNPC::ReceiveNetEvent(Player *pl,UInt32 eventId,NetPacket &packet)
{
	return (SLuaBaseEntity::ImplReceiveNetEvent(pl,eventId,packet) || SBaseNPC::ReceiveNetEvent(pl,eventId,packet)) ? true : false;
}

///////////////////////////////////////////

DEFINE_LUA_ENTITY_WRAPPER_BASE_FUNCTIONS(SLuaNPC);
#endif
