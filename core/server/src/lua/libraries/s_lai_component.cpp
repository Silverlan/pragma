#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/ai/s_factions.h"
#include "luasystem.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/ai/ai_squad.h"
#include "pragma/entities/player.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/model/model.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/entities/components/base_ai_component.hpp>

namespace Lua
{
	namespace NPC
	{
		namespace Server
		{
			static void StartSchedule(lua_State *l,SAIHandle &hEnt,std::shared_ptr<pragma::ai::Schedule> &sched);
			static void CancelSchedule(lua_State *l,SAIHandle &hEnt);
			static void GetSquadName(lua_State *l,SAIHandle &hEnt);
			static void SetRelationship(lua_State *l,SAIHandle &hEnt,const std::string &className,uint32_t disposition,int32_t priority);
			static void SetRelationship(lua_State *l,SAIHandle &hEnt,const std::string &className,uint32_t disposition);
			static void SetRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t disposition,bool revert,int32_t priority);
			static void SetRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t disposition,bool revert);
			static void SetRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t disposition);
			static void SetRelationship(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction,uint32_t disposition,int32_t priority);
			static void SetRelationship(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction,uint32_t disposition);
			static void GetMaxViewDistance(lua_State *l,SAIHandle &hEnt);
			static void SetMaxViewDistance(lua_State *l,SAIHandle &hEnt,float distance);
			static void GetMaxViewAngle(lua_State *l,SAIHandle &hEnt);
			static void SetMaxViewAngle(lua_State *l,SAIHandle &hEnt,float ang);
			static void SetSquad(lua_State *l,SAIHandle &hEnt,const std::string &squadName);
			static void GetNPCState(lua_State *l,SAIHandle &hEnt);
			static void GetSquad(lua_State *l,SAIHandle &hEnt);
			static void ClearRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther);
			static void ClearRelationship(lua_State *l,SAIHandle &hEnt,const std::string &className);
			static void ClearRelationship(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction);
			static void GetDisposition(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther);
			static void GetDisposition(lua_State *l,SAIHandle &hEnt,const std::string &className);
			static void GetDisposition(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction);
			static void GetCurrentSchedule(lua_State *l,SAIHandle &hEnt);
			static void GetMemory(lua_State *l,SAIHandle &hEnt);
			static void GetMemory(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther);
			static void Memorize(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t memType,const Vector3 &pos,const Vector3 &vel);
			static void Memorize(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t memType);
			static void Forget(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther);
			static void ClearMemory(lua_State *l,SAIHandle &hEnt);
			static void IsInMemory(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther);
			static void IsInViewCone(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther);
			static void GetMemoryDuration(lua_State *l,SAIHandle &hEnt);
			static void SetMemoryDuration(lua_State *l,SAIHandle &hEnt,float dur);
			static void CanSee(lua_State *l,SAIHandle &hEnt);
			static void SetHearingStrength(lua_State *l,SAIHandle &hEnt,float strength);
			static void GetHearingStrength(lua_State *l,SAIHandle &hEnt);
			static void CanHear(lua_State *l,SAIHandle &hEnt);
			static void GetMemoryFragmentCount(lua_State *l,SAIHandle &hEnt);
			static void GetPrimaryTarget(lua_State *l,SAIHandle &hEnt);
			static void HasPrimaryTarget(lua_State *l,SAIHandle &hEnt);
			static void SetNPCState(lua_State *l,SAIHandle &hEnt,uint32_t state);
			static void IsMoving(lua_State *l,SAIHandle &hEnt);
			static void IsAIEnabled(lua_State *l,SAIHandle &hEnt);
			static void SetAIEnabled(lua_State *l,SAIHandle &hEnt,bool b);
			static void EnableAI(lua_State *l,SAIHandle &hEnt);
			static void DisableAI(lua_State *l,SAIHandle &hEnt);
			static void IsControllable(lua_State *l,SAIHandle &hEnt);
			static void SetControllable(lua_State *l,SAIHandle &hEnt,bool b);
			static void StartControl(lua_State *l,SAIHandle &hEnt,SPlayerHandle &hPl);
			static void EndControl(lua_State *l,SAIHandle &hEnt);
			static void IsControlled(lua_State *l,SAIHandle &hEnt);
			static void GetController(lua_State *l,SAIHandle &hEnt);
			static void IsEnemy(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther);
			static void LockAnimation(lua_State *l,SAIHandle &hEnt,bool b);
			static void IsAnimationLocked(lua_State *l,SAIHandle &hEnt);

			static void GetDistanceToMoveTarget(lua_State *l,SAIHandle &hEnt);
			static void GetMoveTarget(lua_State *l,SAIHandle &hEnt);
			
			static void MoveTo(lua_State *l,SAIHandle &hEnt,const Vector3 &pos,const pragma::BaseAIComponent::MoveInfo &info);
			static void MoveTo(lua_State *l,SAIHandle &hEnt,const Vector3 &pos);
			static void StopMoving(lua_State *l,SAIHandle &hEnt);
			static void HasReachedDestination(lua_State *l,SAIHandle &hEnt);
			static void GetMoveActivity(lua_State *l,SAIHandle &hEnt);
			static void GetControllerActionInput(lua_State *l,SAIHandle &hEnt);
			static void TriggerScheduleInterrupt(lua_State *l,SAIHandle &hEnt,uint32_t flags);

			static void TurnStep(lua_State *l,SAIHandle &hEnt,const Vector3 &turnTarget,float turnSpeed);
			static void TurnStep(lua_State *l,SAIHandle &hEnt,const Vector3 &turnTarget);
		};
	};
};

void Lua::register_sv_ai_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<SAIHandle,BaseEntityComponentHandle>("AIComponent");
	Lua::register_base_ai_component_methods<luabind::class_<SAIHandle,BaseEntityComponentHandle>,SAIHandle>(l,def);
	def.def("StartSchedule",&Lua::NPC::Server::StartSchedule);
	def.def("CancelSchedule",&Lua::NPC::Server::CancelSchedule);
	def.def("SetSquad",&Lua::NPC::Server::SetSquad);
	def.def("GetSquadName",&Lua::NPC::Server::GetSquadName);
	def.def("SetRelationship",static_cast<void(*)(lua_State*,SAIHandle&,const std::string&,uint32_t,int32_t)>(&Lua::NPC::Server::SetRelationship));
	def.def("SetRelationship",static_cast<void(*)(lua_State*,SAIHandle&,const std::string&,uint32_t)>(&Lua::NPC::Server::SetRelationship));
	def.def("SetRelationship",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&,uint32_t,bool,int32_t)>(&Lua::NPC::Server::SetRelationship));
	def.def("SetRelationship",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&,uint32_t,bool)>(&Lua::NPC::Server::SetRelationship));
	def.def("SetRelationship",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&,uint32_t)>(&Lua::NPC::Server::SetRelationship));
	def.def("SetRelationship",static_cast<void(*)(lua_State*,SAIHandle&,const std::shared_ptr<Faction>&,uint32_t,int32_t)>(&Lua::NPC::Server::SetRelationship));
	def.def("SetRelationship",static_cast<void(*)(lua_State*,SAIHandle&,const std::shared_ptr<Faction>&,uint32_t)>(&Lua::NPC::Server::SetRelationship));
	def.def("GetMaxViewDistance",&Lua::NPC::Server::GetMaxViewDistance);
	def.def("SetMaxViewDistance",&Lua::NPC::Server::SetMaxViewDistance);
	def.def("GetMaxViewAngle",&Lua::NPC::Server::GetMaxViewAngle);
	def.def("SetMaxViewAngle",&Lua::NPC::Server::SetMaxViewAngle);
	def.def("GetSquad",&Lua::NPC::Server::GetSquad);
	def.def("ClearRelationship",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&)>(&Lua::NPC::Server::ClearRelationship));
	def.def("ClearRelationship",static_cast<void(*)(lua_State*,SAIHandle&,const std::string&)>(&Lua::NPC::Server::ClearRelationship));
	def.def("ClearRelationship",static_cast<void(*)(lua_State*,SAIHandle&,const std::shared_ptr<Faction>&)>(&Lua::NPC::Server::ClearRelationship));
	def.def("GetDisposition",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&)>(&Lua::NPC::Server::GetDisposition));
	def.def("GetDisposition",static_cast<void(*)(lua_State*,SAIHandle&,const std::string&)>(&Lua::NPC::Server::GetDisposition));
	def.def("GetDisposition",static_cast<void(*)(lua_State*,SAIHandle&,const std::shared_ptr<Faction>&)>(&Lua::NPC::Server::GetDisposition));
	def.def("GetCurrentSchedule",&Lua::NPC::Server::GetCurrentSchedule);
	def.def("GetMemory",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&)>(&Lua::NPC::Server::GetMemory));
	def.def("GetMemory",static_cast<void(*)(lua_State*,SAIHandle&)>(&Lua::NPC::Server::GetMemory));
	def.def("Memorize",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&,uint32_t,const Vector3&,const Vector3&)>(&Lua::NPC::Server::Memorize));
	def.def("Memorize",static_cast<void(*)(lua_State*,SAIHandle&,EntityHandle&,uint32_t)>(&Lua::NPC::Server::Memorize));
	def.def("Forget",&Lua::NPC::Server::Forget);
	def.def("ClearMemory",&Lua::NPC::Server::ClearMemory);
	def.def("IsInMemory",&Lua::NPC::Server::IsInMemory);
	def.def("IsInViewCone",&Lua::NPC::Server::IsInViewCone);
	def.def("GetMemoryDuration",&Lua::NPC::Server::GetMemoryDuration);
	def.def("SetMemoryDuration",&Lua::NPC::Server::SetMemoryDuration);
	def.def("CanSee",&Lua::NPC::Server::CanSee);
	def.def("SetHearingStrength",&Lua::NPC::Server::SetHearingStrength);
	def.def("GetHearingStrength",&Lua::NPC::Server::GetHearingStrength);
	def.def("CanHear",&Lua::NPC::Server::CanHear);
	def.def("GetMemoryFragmentCount",&Lua::NPC::Server::GetMemoryFragmentCount);
	def.def("GetPrimaryTarget",&Lua::NPC::Server::GetPrimaryTarget);
	def.def("HasPrimaryTarget",&Lua::NPC::Server::HasPrimaryTarget);
	def.def("GetNPCState",&Lua::NPC::Server::GetNPCState);
	def.def("SetNPCState",&Lua::NPC::Server::SetNPCState);
	def.def("IsMoving",&Lua::NPC::Server::IsMoving);
	def.def("IsAIEnabled",&Lua::NPC::Server::IsAIEnabled);
	def.def("SetAIEnabled",&Lua::NPC::Server::SetAIEnabled);
	def.def("EnableAI",&Lua::NPC::Server::EnableAI);
	def.def("DisableAI",&Lua::NPC::Server::DisableAI);
	def.def("IsControllable",&Lua::NPC::Server::IsControllable);
	def.def("SetControllable",&Lua::NPC::Server::SetControllable);
	def.def("StartControl",&Lua::NPC::Server::StartControl);
	def.def("EndControl",&Lua::NPC::Server::EndControl);
	def.def("IsControlled",&Lua::NPC::Server::IsControlled);
	def.def("GetController",&Lua::NPC::Server::GetController);
	def.def("IsEnemy",&Lua::NPC::Server::IsEnemy);
	def.def("LockAnimation",&Lua::NPC::Server::LockAnimation);
	def.def("IsAnimationLocked",&Lua::NPC::Server::IsAnimationLocked);
	def.def("TurnStep",static_cast<void(*)(lua_State*,SAIHandle&,const Vector3&,float)>(&Lua::NPC::Server::TurnStep));
	def.def("TurnStep",static_cast<void(*)(lua_State*,SAIHandle&,const Vector3&)>(&Lua::NPC::Server::TurnStep));

	def.def("GetDistanceToMoveTarget",&Lua::NPC::Server::GetDistanceToMoveTarget);
	def.def("GetMoveTarget",&Lua::NPC::Server::GetMoveTarget);
	def.def("MoveTo",static_cast<void(*)(lua_State*,SAIHandle&,const Vector3&)>(&Lua::NPC::Server::MoveTo));
	def.def("MoveTo",static_cast<void(*)(lua_State*,SAIHandle&,const Vector3&,const pragma::BaseAIComponent::MoveInfo&)>(&Lua::NPC::Server::MoveTo));
	def.def("StopMoving",&Lua::NPC::Server::StopMoving);
	def.def("HasReachedDestination",&Lua::NPC::Server::HasReachedDestination);
	def.def("GetMoveActivity",&Lua::NPC::Server::GetMoveActivity);
	def.def("GetControllerActionInput",&Lua::NPC::Server::GetControllerActionInput);
	def.def("TriggerScheduleInterrupt",&Lua::NPC::Server::TriggerScheduleInterrupt);
	
	def.def("PlayActivity",static_cast<void(*)(lua_State*,SAIHandle&,uint16_t,pragma::SAIComponent::AIAnimationInfo&)>([](lua_State *l,SAIHandle &hEnt,uint16_t activity,pragma::SAIComponent::AIAnimationInfo &info) {
		pragma::Lua::check_component(l,hEnt);
		Lua::PushBool(l,hEnt->PlayActivity(static_cast<Activity>(activity),info));
	}));
	def.def("PlayAnimation",static_cast<void(*)(lua_State*,SAIHandle&,uint16_t,pragma::SAIComponent::AIAnimationInfo&)>([](lua_State *l,SAIHandle &hEnt,uint16_t animation,pragma::SAIComponent::AIAnimationInfo &info) {
		pragma::Lua::check_component(l,hEnt);
		Lua::PushBool(l,hEnt->PlayAnimation(animation,info));
	}));

	auto defAIAnimInfo = luabind::class_<pragma::SAIComponent::AIAnimationInfo>("AnimationInfo");
	defAIAnimInfo.def(luabind::constructor<>());
	defAIAnimInfo.property("flags",static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info) {
		Lua::PushInt(l,umath::to_integral(info.GetPlayFlags()));
	}),static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&,uint32_t)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info,uint32_t flags) {
		info.SetPlayFlags(static_cast<pragma::FPlayAnim>(flags));
	}));
	defAIAnimInfo.property("playAsSchedule",static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info) {
		Lua::PushBool(l,info.ShouldPlayAsSchedule());
	}),static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&,bool)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info,bool playAsSchedule) {
		info.SetPlayAsSchedule(playAsSchedule);
	}));
	defAIAnimInfo.def("SetFacePrimaryTarget",static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info) {
		info.SetFaceTarget(true);
	}));
	defAIAnimInfo.def("ClearFaceTarget",static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info) {
		info.SetFaceTarget(false);
	}));
	defAIAnimInfo.def("SetFaceTarget",static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&,const Vector3&)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info,const Vector3 &pos) {
		info.SetFaceTarget(pos);
	}));
	defAIAnimInfo.def("SetFaceTarget",static_cast<void(*)(lua_State*,pragma::SAIComponent::AIAnimationInfo&,EntityHandle&)>([](lua_State *l,pragma::SAIComponent::AIAnimationInfo &info,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		info.SetFaceTarget(hEnt.get());
	}));
	def.scope[defAIAnimInfo];

	def.add_static_constant("EVENT_SELECT_SCHEDULE",pragma::SAIComponent::EVENT_SELECT_SCHEDULE);
	def.add_static_constant("EVENT_SELECT_CONTROLLER_SCHEDULE",pragma::SAIComponent::EVENT_SELECT_CONTROLLER_SCHEDULE);
	def.add_static_constant("EVENT_ON_SCHEDULE_COMPLETE",pragma::SAIComponent::EVENT_ON_SCHEDULE_COMPLETE);
	def.add_static_constant("EVENT_ON_PRIMARY_TARGET_CHANGED",pragma::SAIComponent::EVENT_ON_PRIMARY_TARGET_CHANGED);
	def.add_static_constant("EVENT_ON_PATH_CHANGED",pragma::SAIComponent::EVENT_ON_PATH_CHANGED);
	def.add_static_constant("EVENT_ON_NPC_STATE_CHANGED",pragma::SAIComponent::EVENT_ON_NPC_STATE_CHANGED);
	def.add_static_constant("EVENT_ON_TARGET_VISIBILITY_LOST",pragma::SAIComponent::EVENT_ON_TARGET_VISIBILITY_LOST);
	def.add_static_constant("EVENT_ON_TARGET_VISIBILITY_REACQUIRED",pragma::SAIComponent::EVENT_ON_TARGET_VISIBILITY_REACQUIRED);
	def.add_static_constant("EVENT_ON_MEMORY_GAINED",pragma::SAIComponent::EVENT_ON_MEMORY_GAINED);
	def.add_static_constant("EVENT_ON_MEMORY_LOST",pragma::SAIComponent::EVENT_ON_MEMORY_LOST);
	def.add_static_constant("EVENT_ON_TARGET_ACQUIRED",pragma::SAIComponent::EVENT_ON_TARGET_ACQUIRED);
	def.add_static_constant("EVENT_ON_SUSPICIOUS_SOUND_HEARED",pragma::SAIComponent::EVENT_ON_SUSPICIOUS_SOUND_HEARED);
	def.add_static_constant("EVENT_ON_CONTROLLER_ACTION_INPUT",pragma::SAIComponent::EVENT_ON_CONTROLLER_ACTION_INPUT);
	def.add_static_constant("EVENT_ON_START_CONTROL",pragma::SAIComponent::EVENT_ON_START_CONTROL);
	def.add_static_constant("EVENT_ON_END_CONTROL",pragma::SAIComponent::EVENT_ON_END_CONTROL);
	def.add_static_constant("EVENT_ON_PATH_NODE_CHANGED",pragma::SAIComponent::EVENT_ON_PATH_NODE_CHANGED);
	def.add_static_constant("EVENT_ON_LOOK_TARGET_CHANGED",pragma::SAIComponent::EVENT_ON_LOOK_TARGET_CHANGED);
	def.add_static_constant("EVENT_ON_SCHEDULE_STARTED",pragma::SAIComponent::EVENT_ON_SCHEDULE_STARTED);
	module[def];
}
void Lua::NPC::Server::StartSchedule(lua_State *l,SAIHandle &hEnt,std::shared_ptr<pragma::ai::Schedule> &sched)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->StartSchedule(sched);
}

void Lua::NPC::Server::CancelSchedule(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->CancelSchedule();
}

void Lua::NPC::Server::GetSquadName(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushString(l,hEnt->GetSquadName());
}

void Lua::NPC::Server::SetRelationship(lua_State *l,SAIHandle &hEnt,const std::string &className,uint32_t disposition,int32_t priority)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetRelationship(className,static_cast<DISPOSITION>(disposition),priority);
}
void Lua::NPC::Server::SetRelationship(lua_State *l,SAIHandle &hEnt,const std::string &className,uint32_t disposition) {SetRelationship(l,hEnt,className,disposition,0);}
void Lua::NPC::Server::SetRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t disposition,bool revert,int32_t priority)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	hEnt.get()->SetRelationship(hEntOther,static_cast<DISPOSITION>(disposition),revert,priority);
}
void Lua::NPC::Server::SetRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t disposition,bool revert) {SetRelationship(l,hEnt,hEntOther,disposition,revert,0);}
void Lua::NPC::Server::SetRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t disposition) {SetRelationship(l,hEnt,hEntOther,disposition,true,0);}
void Lua::NPC::Server::SetRelationship(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction,uint32_t disposition,int32_t priority)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetRelationship(*faction,static_cast<DISPOSITION>(disposition),priority);
}
void Lua::NPC::Server::SetRelationship(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction,uint32_t disposition) {SetRelationship(l,hEnt,faction,disposition,0);}
void Lua::NPC::Server::GetMaxViewDistance(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetMaxViewDistance());
}
void Lua::NPC::Server::SetMaxViewDistance(lua_State *l,SAIHandle &hEnt,float distance)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxViewDistance(distance);
}
void Lua::NPC::Server::GetMaxViewAngle(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetMaxViewAngle());
}
void Lua::NPC::Server::SetMaxViewAngle(lua_State *l,SAIHandle &hEnt,float ang)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxViewAngle(ang);
}
void Lua::NPC::Server::SetSquad(lua_State *l,SAIHandle &hEnt,const std::string &squadName)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetSquad(squadName);
}
void Lua::NPC::Server::GetSquad(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto &squad = hEnt->GetSquad();
	if(squad == nullptr)
		return;
	Lua::Push(l,squad);
}
void Lua::NPC::Server::ClearRelationship(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	hEnt->ClearRelationship(hEntOther);
}
void Lua::NPC::Server::ClearRelationship(lua_State *l,SAIHandle &hEnt,const std::string &className)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->ClearRelationship(className);
}
void Lua::NPC::Server::ClearRelationship(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->ClearRelationship(*faction);
}
void Lua::NPC::Server::GetDisposition(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	int32_t priority = 0;
	auto disp = hEnt->GetDisposition(hEntOther,&priority);
	Lua::PushInt(l,umath::to_integral(disp));
	Lua::PushInt(l,priority);
}
void Lua::NPC::Server::GetDisposition(lua_State *l,SAIHandle &hEnt,const std::string &className)
{
	pragma::Lua::check_component(l,hEnt);
	int32_t priority = 0;
	auto disp = hEnt->GetDisposition(className,&priority);
	Lua::PushInt(l,umath::to_integral(disp));
	Lua::PushInt(l,priority);
}
void Lua::NPC::Server::GetDisposition(lua_State *l,SAIHandle &hEnt,const std::shared_ptr<Faction> &faction)
{
	pragma::Lua::check_component(l,hEnt);
	int32_t priority = 0;
	auto disp = hEnt->GetDisposition(*faction,&priority);
	Lua::PushInt(l,umath::to_integral(disp));
	Lua::PushInt(l,priority);
}
void Lua::NPC::Server::GetCurrentSchedule(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto sched = hEnt->GetCurrentSchedule();
	if(sched == nullptr)
		return;
	Lua::Push(l,sched);
}
void Lua::NPC::Server::GetMemory(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto &mem = hEnt->GetMemory();
	Lua::Push<pragma::ai::Memory*>(l,&mem);
}
void Lua::NPC::Server::GetMemory(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	auto *fragment = hEnt->GetMemory(hEntOther.get());
	if(fragment == nullptr)
		return;
	Lua::Push<pragma::ai::Memory::Fragment*>(l,fragment);
}
void Lua::NPC::Server::Memorize(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t memType,const Vector3 &pos,const Vector3 &vel)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	hEnt->Memorize(hEntOther.get(),static_cast<pragma::ai::Memory::MemoryType>(memType),pos,vel);
}
void Lua::NPC::Server::Memorize(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther,uint32_t memType)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	hEnt->Memorize(hEntOther.get(),static_cast<pragma::ai::Memory::MemoryType>(memType));
}
void Lua::NPC::Server::Forget(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	hEnt->Forget(hEntOther.get());
}
void Lua::NPC::Server::ClearMemory(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->ClearMemory();
}
void Lua::NPC::Server::IsInMemory(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	Lua::PushBool(l,hEnt->IsInMemory(hEntOther.get()));
}
void Lua::NPC::Server::IsInViewCone(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	auto dist = 0.f;
	auto r = hEnt->IsInViewCone(hEntOther.get(),&dist);
	Lua::PushBool(l,r);
	if(r == true)
		Lua::PushNumber(l,dist);
}
void Lua::NPC::Server::GetMemoryDuration(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetMemoryDuration());
}
void Lua::NPC::Server::SetMemoryDuration(lua_State *l,SAIHandle &hEnt,float dur)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMemoryDuration(dur);
}
void Lua::NPC::Server::CanSee(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->CanSee());
}
void Lua::NPC::Server::SetHearingStrength(lua_State *l,SAIHandle &hEnt,float strength)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetHearingStrength(strength);
}
void Lua::NPC::Server::GetHearingStrength(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetHearingStrength());
}
void Lua::NPC::Server::CanHear(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->CanHear());
}
void Lua::NPC::Server::GetMemoryFragmentCount(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushInt(l,hEnt->GetMemoryFragmentCount());
}
void Lua::NPC::Server::GetPrimaryTarget(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto *fragment = hEnt->GetPrimaryTarget();
	if(fragment == nullptr)
		return;
	Lua::Push<pragma::ai::Memory::Fragment*>(l,const_cast<pragma::ai::Memory::Fragment*>(fragment));
}
void Lua::NPC::Server::HasPrimaryTarget(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto *fragment = hEnt->GetPrimaryTarget();
	if(fragment == nullptr)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,true);
}
void Lua::NPC::Server::GetNPCState(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushInt(l,umath::to_integral(hEnt->GetNPCState()));
}
void Lua::NPC::Server::SetNPCState(lua_State *l,SAIHandle &hEnt,uint32_t state)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetNPCState(static_cast<NPCSTATE>(state));
}
void Lua::NPC::Server::IsMoving(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsMoving());
}
void Lua::NPC::Server::IsAIEnabled(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsAIEnabled());
}
void Lua::NPC::Server::SetAIEnabled(lua_State *l,SAIHandle &hEnt,bool b)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetAIEnabled(b);
}
void Lua::NPC::Server::EnableAI(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->EnableAI();
}
void Lua::NPC::Server::DisableAI(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->DisableAI();
}
void Lua::NPC::Server::IsControllable(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsControllable());
}
void Lua::NPC::Server::SetControllable(lua_State *l,SAIHandle &hEnt,bool b)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetControllable(b);
}
void Lua::NPC::Server::StartControl(lua_State *l,SAIHandle &hEnt,SPlayerHandle &hPl)
{
	pragma::Lua::check_component(l,hEnt);
	pragma::Lua::check_component(l,hPl);
	hEnt->StartControl(*hPl);
}
void Lua::NPC::Server::EndControl(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->EndControl();
}
void Lua::NPC::Server::IsControlled(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsControlled());
}
void Lua::NPC::Server::GetController(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto *controller = hEnt->GetController();
	if(controller == nullptr)
		return;
	controller->GetEntity().GetLuaObject()->push(l);
}
void Lua::NPC::Server::IsEnemy(lua_State *l,SAIHandle &hEnt,EntityHandle &hEntOther)
{
	pragma::Lua::check_component(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	Lua::PushBool(l,hEnt->IsEnemy(hEntOther.get()));
}
void Lua::NPC::Server::GetDistanceToMoveTarget(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetDistanceToMoveTarget());
}
void Lua::NPC::Server::GetMoveTarget(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::Push<Vector3>(l,hEnt->GetMoveTarget());
}
void Lua::NPC::Server::MoveTo(lua_State *l,SAIHandle &hEnt,const Vector3 &pos,const pragma::BaseAIComponent::MoveInfo &info)
{
	pragma::Lua::check_component(l,hEnt);
	auto r = hEnt->MoveTo(pos,info);
	Lua::PushInt(l,umath::to_integral(r));
}
void Lua::NPC::Server::MoveTo(lua_State *l,SAIHandle &hEnt,const Vector3 &pos) {MoveTo(l,hEnt,pos,{});}
void Lua::NPC::Server::StopMoving(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->StopMoving();
}
void Lua::NPC::Server::HasReachedDestination(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->HasReachedDestination());
}
void Lua::NPC::Server::GetMoveActivity(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushInt(l,umath::to_integral(hEnt->GetMoveActivity()));
}
void Lua::NPC::Server::GetControllerActionInput(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushInt(l,umath::to_integral(hEnt->GetControllerActionInput()));
}
void Lua::NPC::Server::TriggerScheduleInterrupt(lua_State *l,SAIHandle &hEnt,uint32_t flags)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->TriggerScheduleInterrupt(flags));
}
void Lua::NPC::Server::LockAnimation(lua_State *l,SAIHandle &hEnt,bool b)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->LockAnimation(b);
}
void Lua::NPC::Server::IsAnimationLocked(lua_State *l,SAIHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsAnimationLocked());
}
void Lua::NPC::Server::TurnStep(lua_State *l,SAIHandle &hEnt,const Vector3 &turnTarget,float turnSpeed)
{
	pragma::Lua::check_component(l,hEnt);
	auto turnAngle = 0.f;
	auto r = hEnt->TurnStep(turnTarget,turnAngle,&turnSpeed);
	Lua::PushBool(l,r);
	Lua::PushNumber(l,turnAngle);
}
void Lua::NPC::Server::TurnStep(lua_State *l,SAIHandle &hEnt,const Vector3 &turnTarget)
{
	pragma::Lua::check_component(l,hEnt);
	auto turnAngle = 0.f;
	auto r = hEnt->TurnStep(turnTarget,turnAngle);
	Lua::PushBool(l,r);
	Lua::PushNumber(l,turnAngle);
}
