#include "stdafx_server.h"
#include "luasystem.h"
#include "pragma/lua/libraries/s_lai.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/lua/classes/s_lai_behavior.h"
#include "pragma/ai/s_disposition.h"
#include "pragma/ai/s_npcstate.h"
#include "pragma/ai/ai_task.h"
#include "pragma/ai/ai_task_decorator.h"
#include "pragma/lua/classes/s_laimemory.h"
#include "pragma/lua/classes/s_laisquad.h"
#include "pragma/lua/classes/s_lfaction.h"
#include "pragma/lua/classes/s_lai_schedule.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/ai/ai_memory.h"
#include "pragma/ai/ai_task_move_to_target.h"
#include "pragma/ai/ai_task_move_random.h"
#include "pragma/ai/ai_task_look_at_target.h"
#include "pragma/ai/ai_task_play_sound.h"
#include "pragma/ai/ai_task_random.h"
#include "pragma/ai/ai_task_play_animation.h"
#include "pragma/ai/ai_task_play_activity.h"
#include "pragma/ai/ai_task_debug.h"
#include "pragma/ai/ai_task_event.hpp"
#include "pragma/ai/ai_task_wait.h"
#include <pragma/lua/lua_shared_ptr_converter.hpp>
#include <luainterface.hpp>
#include <pragma/lua/libraries/lai.hpp>

extern DLLSERVER SGame *s_game;

namespace Lua
{
	namespace ai
	{
		static int create_schedule(lua_State *l);
		static int register_faction(lua_State *l);
		static int get_factions(lua_State *l);
		static int find_faction_by_name(lua_State *l);
		static int register_task(lua_State *l);
	};
};

void Lua::ai::server::register_library(Lua::Interface &lua)
{
	auto &modAi = lua.RegisterLibrary("ai",{
		{"create_schedule",create_schedule},
		{"register_faction",register_faction},
		{"get_factions",get_factions},
		{"find_faction_by_name",find_faction_by_name},
		{"register_task",register_task}
	});
	Lua::ai::register_library(lua);

	auto *l = lua.GetState();
	Lua::RegisterLibraryEnums(l,"ai",{
		{"DISPOSITION_HATE",umath::to_integral(DISPOSITION::HATE)},
		{"DISPOSITION_FEAR",umath::to_integral(DISPOSITION::FEAR)},
		{"DISPOSITION_NEUTRAL",umath::to_integral(DISPOSITION::NEUTRAL)},
		{"DISPOSITION_LIKE",umath::to_integral(DISPOSITION::LIKE)},

		{"NPC_STATE_NONE",umath::to_integral(NPCSTATE::NONE)},
		{"NPC_STATE_IDLE",umath::to_integral(NPCSTATE::IDLE)},
		{"NPC_STATE_ALERT",umath::to_integral(NPCSTATE::ALERT)},
		{"NPC_STATE_COMBAT",umath::to_integral(NPCSTATE::COMBAT)},
		{"NPC_STATE_SCRIPT",umath::to_integral(NPCSTATE::SCRIPT)},

		{"TASK_MOVE_TO_TARGET",umath::to_integral(::pragma::ai::Task::MoveToTarget)},
		{"TASK_PLAY_ANIMATION",umath::to_integral(::pragma::ai::Task::PlayAnimation)},
		{"TASK_PLAY_ACTIVITY",umath::to_integral(::pragma::ai::Task::PlayActivity)},
		{"TASK_PLAY_LAYERED_ANIMATION",umath::to_integral(::pragma::ai::Task::PlayLayeredAnimation)},
		{"TASK_PLAY_LAYERED_ACTIVITY",umath::to_integral(::pragma::ai::Task::PlayLayeredActivity)},
		{"TASK_MOVE_RANDOM",umath::to_integral(::pragma::ai::Task::MoveRandom)},
		{"TASK_PLAY_SOUND",umath::to_integral(::pragma::ai::Task::PlaySound)},
		{"TASK_DEBUG_PRINT",umath::to_integral(::pragma::ai::Task::DebugPrint)},
		{"TASK_DEBUG_DRAW_TEXT",umath::to_integral(::pragma::ai::Task::DebugDrawText)},
		{"TASK_DECORATOR",umath::to_integral(::pragma::ai::Task::Decorator)},
		{"TASK_WAIT",umath::to_integral(::pragma::ai::Task::Wait)},
		{"TASK_TURN_TO_TARGET",umath::to_integral(::pragma::ai::Task::TurnToTarget)},
		{"TASK_RANDOM",umath::to_integral(::pragma::ai::Task::Random)},
		{"TASK_LOOK_AT_TARGET",umath::to_integral(::pragma::ai::Task::LookAtTarget)},
		{"TASK_EVENT",umath::to_integral(::pragma::ai::Task::Event)},
		
		{"MOVE_STATE_TARGET_UNREACHABLE",umath::to_integral(pragma::SAIComponent::MoveResult::TargetUnreachable)},
		{"MOVE_STATE_TARGET_REACHED",umath::to_integral(pragma::SAIComponent::MoveResult::TargetReached)},
		{"MOVE_STATE_WAITING_FOR_PATH",umath::to_integral(pragma::SAIComponent::MoveResult::WaitingForPath)},
		{"MOVE_STATE_MOVING_TO_TARGET",umath::to_integral(pragma::SAIComponent::MoveResult::MovingToTarget)}
	});
	
	Lua::AISquad::register_class(l,modAi);
	Lua::AIMemory::register_class(l,modAi);
	Lua::Faction::register_class(l,modAi);
	Lua::AISchedule::register_class(l,modAi);
	Lua::AIBehaviorNode::register_class(l,modAi);

	auto cdMoveToTarget = luabind::class_<TaskWrapperMoveToTarget,Lua::ai::TaskWrapper>("TaskMoveToTarget");
	cdMoveToTarget.add_static_constant("PARAMETER_TARGET",umath::to_integral(pragma::ai::TaskMoveToTarget::Parameter::Target));
	cdMoveToTarget.add_static_constant("PARAMETER_DISTANCE",umath::to_integral(pragma::ai::TaskMoveToTarget::Parameter::Distance));
	cdMoveToTarget.add_static_constant("PARAMETER_MOVE_ACTIVITY",umath::to_integral(pragma::ai::TaskMoveToTarget::Parameter::MoveActivity));
	cdMoveToTarget.def("SetMoveTarget",static_cast<void(*)(lua_State*,TaskWrapperMoveToTarget&,const Vector3&)>([](lua_State *l,TaskWrapperMoveToTarget &task,const Vector3 &target) {
		task->SetTarget(target);
	}));
	cdMoveToTarget.def("SetMoveTarget",static_cast<void(*)(lua_State*,TaskWrapperMoveToTarget&,EntityHandle&)>([](lua_State *l,TaskWrapperMoveToTarget &task,EntityHandle &hEnt) {
		lua_checkentity(l,(&hEnt));
		task->SetTarget(hEnt.get());
	}));
	cdMoveToTarget.def("SetMoveDistance",static_cast<void(*)(lua_State*,TaskWrapperMoveToTarget&,float)>([](lua_State *l,TaskWrapperMoveToTarget &task,float dist) {
		task->SetMoveDistance(dist);
	}));
	cdMoveToTarget.def("SetMoveActivity",static_cast<void(*)(lua_State*,TaskWrapperMoveToTarget&,std::underlying_type_t<Activity>)>([](lua_State *l,TaskWrapperMoveToTarget &task,std::underlying_type_t<Activity> activity) {
		task->SetMoveActivity(static_cast<Activity>(activity));
	}));
	modAi[cdMoveToTarget];

	auto cdMoveRandom = luabind::class_<TaskWrapperMoveRandom,Lua::ai::TaskWrapper>("TaskMoveRandom");
	cdMoveRandom.add_static_constant("PARAMETER_DISTANCE",umath::to_integral(pragma::ai::TaskMoveRandom::Parameter::Distance));
	cdMoveRandom.add_static_constant("PARAMETER_MOVE_ACTIVITY",umath::to_integral(pragma::ai::TaskMoveRandom::Parameter::MoveActivity));
	cdMoveRandom.def("SetMoveDistance",static_cast<void(*)(lua_State*,TaskWrapperMoveRandom&,float)>([](lua_State *l,TaskWrapperMoveRandom &task,float dist) {
		task->SetMoveDistance(dist);
	}));
	cdMoveRandom.def("SetMoveActivity",static_cast<void(*)(lua_State*,TaskWrapperMoveRandom&,std::underlying_type_t<Activity>)>([](lua_State *l,TaskWrapperMoveRandom &task,std::underlying_type_t<Activity> activity) {
		task->SetMoveActivity(static_cast<Activity>(activity));
	}));
	modAi[cdMoveRandom];
	
	auto cdLookAtTarget = luabind::class_<TaskWrapperLookAtTarget,Lua::ai::TaskWrapper>("TaskLookAtTarget");
	cdLookAtTarget.add_static_constant("PARAMETER_LOOK_DURATION",umath::to_integral(pragma::ai::TaskLookAtTarget::Parameter::LookDuration));
	cdLookAtTarget.def("SetLookDuration",static_cast<void(*)(lua_State*,TaskWrapperLookAtTarget&,float)>([](lua_State *l,TaskWrapperLookAtTarget &task,float duration) {
		task->SetLookDuration(duration);
	}));
	modAi[cdLookAtTarget];
	
	auto cdPlayAnimation = luabind::class_<TaskWrapperPlayAnimation,Lua::ai::TaskWrapper>("TaskPlayAnimation");
	cdPlayAnimation.add_static_constant("PARAMETER_ANIMATION",umath::to_integral(pragma::ai::TaskPlayAnimation::Parameter::Animation));
	cdPlayAnimation.add_static_constant("PARAMETER_FACE_TARGET",umath::to_integral(pragma::ai::TaskPlayAnimation::Parameter::FaceTarget));
	cdPlayAnimation.def("SetAnimation",static_cast<void(*)(lua_State*,TaskWrapperPlayAnimation&,int32_t)>([](lua_State *l,TaskWrapperPlayAnimation &task,int32_t animation) {
		task->SetAnimation(animation);
	}));
	cdPlayAnimation.def("SetAnimation",static_cast<void(*)(lua_State*,TaskWrapperPlayAnimation&,const std::string&)>([](lua_State *l,TaskWrapperPlayAnimation &task,const std::string &animation) {
		task->SetAnimation(animation);
	}));
	cdPlayAnimation.def("SetFaceTarget",static_cast<void(*)(lua_State*,TaskWrapperPlayAnimation&,const Vector3&)>([](lua_State *l,TaskWrapperPlayAnimation &task,const Vector3 &target) {
		task->SetFaceTarget(target);
	}));
	cdPlayAnimation.def("SetFaceTarget",static_cast<void(*)(lua_State*,TaskWrapperPlayAnimation&,EntityHandle&)>([](lua_State *l,TaskWrapperPlayAnimation &task,EntityHandle &target) {
		lua_checkentity(l,(&target));
		task->SetFaceTarget(*target.get());
	}));
	cdPlayAnimation.def("SetFacePrimaryTarget",static_cast<void(*)(lua_State*,TaskWrapperPlayAnimation&)>([](lua_State *l,TaskWrapperPlayAnimation &task) {
		task->SetFacePrimaryTarget();
	}));
	modAi[cdPlayAnimation];
	
	auto cdPlayActivity = luabind::class_<TaskWrapperPlayActivity,Lua::ai::TaskWrapper>("TaskPlayActivity");
	cdPlayActivity.add_static_constant("PARAMETER_ACTIVITY",umath::to_integral(pragma::ai::TaskPlayActivity::Parameter::Activity));
	cdPlayActivity.add_static_constant("PARAMETER_FACE_TARGET",umath::to_integral(pragma::ai::TaskPlayActivity::Parameter::FaceTarget));
	cdPlayActivity.def("SetActivity",static_cast<void(*)(lua_State*,TaskWrapperPlayActivity&,std::underlying_type_t<Activity>)>([](lua_State *l,TaskWrapperPlayActivity &task,std::underlying_type_t<Activity> activity) {
		task->SetActivity(static_cast<Activity>(activity));
	}));
	cdPlayActivity.def("SetFaceTarget",static_cast<void(*)(lua_State*,TaskWrapperPlayActivity&,const Vector3&)>([](lua_State *l,TaskWrapperPlayActivity &task,const Vector3 &target) {
		task->SetFaceTarget(target);
	}));
	cdPlayActivity.def("SetFaceTarget",static_cast<void(*)(lua_State*,TaskWrapperPlayActivity&,EntityHandle&)>([](lua_State *l,TaskWrapperPlayActivity &task,EntityHandle &target) {
		lua_checkentity(l,(&target));
		task->SetFaceTarget(*target.get());
	}));
	cdPlayActivity.def("SetFacePrimaryTarget",static_cast<void(*)(lua_State*,TaskWrapperPlayActivity&)>([](lua_State *l,TaskWrapperPlayActivity &task) {
		task->SetFacePrimaryTarget();
	}));
	modAi[cdPlayActivity];

	auto cdPlayLayeredAnimation = luabind::class_<TaskWrapperPlayLayeredAnimation,Lua::ai::TaskWrapper>("TaskPlayLayeredAnimation");
	cdPlayLayeredAnimation.add_static_constant("PARAMETER_ANIMATION",umath::to_integral(pragma::ai::TaskPlayLayeredAnimation::Parameter::Animation));
	cdPlayLayeredAnimation.add_static_constant("PARAMETER_ANIMATION_SLOT",umath::to_integral(pragma::ai::TaskPlayLayeredAnimation::Parameter::AnimationSlot));
	cdPlayLayeredAnimation.def("SetAnimation",static_cast<void(*)(lua_State*,TaskWrapperPlayLayeredAnimation&,int32_t)>([](lua_State *l,TaskWrapperPlayLayeredAnimation &task,int32_t animation) {
		task->SetAnimation(animation);
	}));
	cdPlayLayeredAnimation.def("SetAnimation",static_cast<void(*)(lua_State*,TaskWrapperPlayLayeredAnimation&,const std::string&)>([](lua_State *l,TaskWrapperPlayLayeredAnimation &task,const std::string &animation) {
		task->SetAnimation(animation);
	}));
	cdPlayLayeredAnimation.def("SetAnimationSlot",static_cast<void(*)(lua_State*,TaskWrapperPlayLayeredAnimation&,int32_t)>([](lua_State *l,TaskWrapperPlayLayeredAnimation &task,int32_t animationSlot) {
		task->SetAnimationSlot(animationSlot);
	}));
	modAi[cdPlayLayeredAnimation];

	auto cdPlayLayeredActivity = luabind::class_<TaskWrapperPlayLayeredActivity,Lua::ai::TaskWrapper>("TaskPlayLayeredActivity");
	cdPlayLayeredActivity.add_static_constant("PARAMETER_ACTIVITY",umath::to_integral(pragma::ai::TaskPlayLayeredActivity::Parameter::Activity));
	cdPlayLayeredActivity.add_static_constant("PARAMETER_ANIMATION_SLOT",umath::to_integral(pragma::ai::TaskPlayLayeredActivity::Parameter::AnimationSlot));
	cdPlayLayeredActivity.def("SetActivity",static_cast<void(*)(lua_State*,TaskWrapperPlayLayeredActivity&,std::underlying_type_t<Activity>)>([](lua_State *l,TaskWrapperPlayLayeredActivity &task,std::underlying_type_t<Activity> activity) {
		task->SetActivity(static_cast<Activity>(activity));
	}));
	cdPlayLayeredActivity.def("SetAnimationSlot",static_cast<void(*)(lua_State*,TaskWrapperPlayLayeredActivity&,int32_t)>([](lua_State *l,TaskWrapperPlayLayeredActivity &task,int32_t animationSlot) {
		task->SetAnimationSlot(animationSlot);
	}));
	modAi[cdPlayLayeredActivity];

	auto cdPlaySound = luabind::class_<TaskWrapperPlaySound,Lua::ai::TaskWrapper>("TaskPlaySound");
	cdPlaySound.add_static_constant("PARAMETER_SOUND_NAME",umath::to_integral(pragma::ai::TaskPlaySound::Parameter::SoundName));
	cdPlaySound.add_static_constant("PARAMETER_GAIN",umath::to_integral(pragma::ai::TaskPlaySound::Parameter::Gain));
	cdPlaySound.add_static_constant("PARAMETER_PITCH",umath::to_integral(pragma::ai::TaskPlaySound::Parameter::Pitch));
	cdPlaySound.def("SetSoundName",static_cast<void(*)(lua_State*,TaskWrapperPlaySound&,const std::string&)>([](lua_State *l,TaskWrapperPlaySound &task,const std::string &sndName) {
		task->SetSoundName(sndName);
	}));
	cdPlaySound.def("SetGain",static_cast<void(*)(lua_State*,TaskWrapperPlaySound&,float)>([](lua_State *l,TaskWrapperPlaySound &task,float gain) {
		task->SetGain(gain);
	}));
	cdPlaySound.def("SetPitch",static_cast<void(*)(lua_State*,TaskWrapperPlaySound&,float)>([](lua_State *l,TaskWrapperPlaySound &task,float pitch) {
		task->SetPitch(pitch);
	}));
	modAi[cdPlaySound];

	auto cdRandom = luabind::class_<TaskWrapperRandom,Lua::ai::TaskWrapper>("TaskRandom");
	cdRandom.add_static_constant("PARAMETER_CHANCE",umath::to_integral(pragma::ai::TaskRandom::Parameter::Chance));
	cdRandom.def("SetChance",static_cast<void(*)(lua_State*,TaskWrapperRandom&,float)>([](lua_State *l,TaskWrapperRandom &task,float chance) {
		task->SetChance(chance);
	}));
	modAi[cdRandom];

	auto cdDebugPrint = luabind::class_<TaskWrapperDebugPrint,Lua::ai::TaskWrapper>("TaskDebugPrint");
	cdDebugPrint.add_static_constant("PARAMETER_MESSAGE",umath::to_integral(pragma::ai::TaskDebugPrint::Parameter::Message));
	cdDebugPrint.def("SetMessage",static_cast<void(*)(lua_State*,TaskWrapperDebugPrint&,const std::string&)>([](lua_State *l,TaskWrapperDebugPrint &task,const std::string &msg) {
		task->SetMessage(msg);
	}));
	modAi[cdDebugPrint];
	
	auto cdDebugDrawText = luabind::class_<TaskWrapperDebugDrawText,Lua::ai::TaskWrapper>("TaskDebugDrawText");
	cdDebugDrawText.add_static_constant("PARAMETER_MESSAGE",umath::to_integral(pragma::ai::TaskDebugDrawText::Parameter::Message));
	cdDebugDrawText.def("SetMessage",static_cast<void(*)(lua_State*,TaskWrapperDebugDrawText&,const std::string&)>([](lua_State *l,TaskWrapperDebugDrawText &task,const std::string &msg) {
		task->SetMessage(msg);
	}));
	modAi[cdDebugDrawText];

	auto cdDecorator = luabind::class_<TaskWrapperDecorator,Lua::ai::TaskWrapper>("TaskDecorator");
	cdDecorator.add_static_constant("PARAMETER_DECORATOR_TYPE",umath::to_integral(pragma::ai::TaskDecorator::Parameter::DecoratorType));
	cdDecorator.add_static_constant("PARAMETER_LIMIT",umath::to_integral(pragma::ai::TaskDecorator::Parameter::Limit));
	cdDecorator.def("SetDecoratorType",static_cast<void(*)(lua_State*,TaskWrapperDecorator&,std::underlying_type_t<pragma::ai::TaskDecorator::DecoratorType>)>([](lua_State *l,TaskWrapperDecorator &task,std::underlying_type_t<pragma::ai::TaskDecorator::DecoratorType> decoratorType) {
		task->SetDecoratorType(static_cast<pragma::ai::TaskDecorator::DecoratorType>(decoratorType));
	}));
	cdDecorator.def("SetLimit",static_cast<void(*)(lua_State*,TaskWrapperDecorator&,uint32_t)>([](lua_State *l,TaskWrapperDecorator &task,uint32_t limit) {
		task->SetLimit(limit);
	}));
	modAi[cdDecorator];

	auto cdEvent = luabind::class_<TaskWrapperEvent,Lua::ai::TaskWrapper>("TaskEvent");
	cdEvent.add_static_constant("PARAMETER_EVENT_ID",umath::to_integral(pragma::ai::TaskEvent::Parameter::EventId));
	cdEvent.add_static_constant("PARAMETER_EVENT_ARGS_START",umath::to_integral(pragma::ai::TaskEvent::Parameter::EventArgStart));
	cdEvent.def("SetEventId",static_cast<void(*)(lua_State*,TaskWrapperEvent&,std::underlying_type_t<AnimationEvent::Type>)>([](lua_State *l,TaskWrapperEvent &task,std::underlying_type_t<AnimationEvent::Type> eventId) {
		task->SetEventId(static_cast<AnimationEvent::Type>(eventId));
	}));
	cdEvent.def("SetEventArgument",static_cast<void(*)(lua_State*,TaskWrapperEvent&,uint32_t,const std::string&)>([](lua_State *l,TaskWrapperEvent &task,uint32_t argIdx,const std::string &arg) {
		task->SetEventArgument(argIdx,arg);
	}));
	modAi[cdEvent];

	auto cdWait = luabind::class_<TaskWrapperWait,Lua::ai::TaskWrapper>("TaskWait");
	cdWait.add_static_constant("PARAMETER_MIN_WAIT_TIME",umath::to_integral(pragma::ai::TaskWait::Parameter::MinWaitTime));
	cdWait.add_static_constant("PARAMETER_MAX_WAIT_TIME",umath::to_integral(pragma::ai::TaskWait::Parameter::MaxWaitTime));
	cdWait.def("SetMinWaitTime",static_cast<void(*)(lua_State*,TaskWrapperWait&,float)>([](lua_State *l,TaskWrapperWait &task,float t) {
		task->SetMinWaitTime(t);
	}));
	cdWait.def("SetMaxWaitTime",static_cast<void(*)(lua_State*,TaskWrapperWait&,float)>([](lua_State *l,TaskWrapperWait &task,float t) {
		task->SetMaxWaitTime(t);
	}));
	cdWait.def("SetWaitTime",static_cast<void(*)(lua_State*,TaskWrapperWait&,float,float)>([](lua_State *l,TaskWrapperWait &task,float tMin,float tMax) {
		task->SetWaitTime(tMin,tMax);
	}));
	cdWait.def("SetWaitTime",static_cast<void(*)(lua_State*,TaskWrapperWait&,float)>([](lua_State *l,TaskWrapperWait &task,float t) {
		task->SetWaitTime(t,t);
	}));
	modAi[cdWait];
}

std::shared_ptr<::pragma::ai::BehaviorNode> Lua::ai::server::create_lua_task(lua_State *l,luabind::object oClass,::pragma::ai::BehaviorNode::Type taskType,::pragma::ai::SelectorType selectorType)
{
	luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
	try
	{
#endif
		r = oClass(umath::to_integral(taskType),umath::to_integral(selectorType));
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error&)
	{
		Lua::HandleLuaError(l);
		return nullptr;
	}
#endif
	if(!r)
	{
		Con::csv<<"WARNING: Unable to create lua AI Task!"<<Con::endl;
		return nullptr;
	}
	auto *ptr = luabind::object_cast_nothrow<AILuaBehaviorNodeWrapper*>(r,static_cast<AILuaBehaviorNodeWrapper*>(nullptr));
	if(ptr)
	{
		auto &luaNode = static_cast<AILuaBehaviorNode&>(ptr->GetTask());
		luaNode.SetLuaObject(r);//Lua::CreateWeakReference(r)); // TODO: This works if the task is attached to another task, but what if it's not? -> DANGLING REFERENCE!!!
		luaNode.SetLuaClass(oClass);
		return std::static_pointer_cast<::pragma::ai::BehaviorNode>(luaNode.shared_from_this());
	}
	Con::csv<<"WARNING: Unable to create lua AI Task: Lua class is not derived from valid AI Task base!"<<Con::endl;
	return nullptr;
}

int Lua::ai::create_schedule(lua_State *l)
{
	auto sched = ::pragma::ai::Schedule::Create();
	Lua::Push(l,sched);
	return 1;
}

int Lua::ai::register_faction(lua_State *l)
{
	auto *name = Lua::CheckString(l,1);
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	auto faction = factionManager.RegisterFaction(name);
	Lua::Push(l,faction);
	return 1;
}
int Lua::ai::get_factions(lua_State *l)
{
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	auto &factions = factionManager.GetFactions();
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(factions.size()){0};i<factions.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::Push(l,factions[i]);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::ai::find_faction_by_name(lua_State *l)
{
	auto *name = Lua::CheckString(l,1);
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	auto faction = factionManager.FindFactionByName(name);
	if(faction == nullptr)
		return 0;
	Lua::Push(l,faction);
	return 1;
}

int Lua::ai::register_task(lua_State *l)
{
	luaL_checkuserdata(l,1);
	auto o = luabind::object(luabind::from_stack(l,1)); // Class Definition
	auto taskType = ::pragma::ai::BehaviorNode::Type::Sequence;
	auto selectorType = ::pragma::ai::SelectorType::Sequential;
	if(Lua::IsSet(l,2) == true)
		taskType = static_cast<::pragma::ai::BehaviorNode::Type>(Lua::CheckInt(l,2));
	if(Lua::IsSet(l,3) == true)
		selectorType = static_cast<::pragma::ai::SelectorType>(Lua::CheckInt(l,3));
	auto &taskManager = s_game->GetAITaskManager();
	auto taskId = taskManager.RegisterTask(std::bind(server::create_lua_task,l,o,taskType,selectorType));
	Lua::PushInt(l,taskId);
	return 1;
}
