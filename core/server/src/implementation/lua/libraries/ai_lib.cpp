// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.ai;

import :ai;
import :entities.components;
import :game;
import :scripting.lua.classes.ai_behavior;
import :scripting.lua.classes.ai_memory;
import :scripting.lua.classes.ai_schedule;
import :scripting.lua.classes.ai_squad;
import :scripting.lua.classes.faction;

#undef PlaySound

using LuaFactionObject = luabind::object;
namespace Lua {
	namespace ai {
		static std::shared_ptr<pragma::ai::Schedule> create_schedule();
		static std::shared_ptr<::Faction> register_faction(const std::string &name);
		static LuaTableObject get_factions(lua::State *l);
		static LuaFactionObject find_faction_by_name(lua::State *l, const std::string &name);
		static uint32_t register_task(lua::State *l, const LuaClassObject &taskClass, pragma::ai::BehaviorNode::Type taskType, pragma::ai::SelectorType selectorType);
		static uint32_t register_task(lua::State *l, const LuaClassObject &taskClass, pragma::ai::BehaviorNode::Type taskType);
		static uint32_t register_task(lua::State *l, const LuaClassObject &taskClass);
	};
};

void Lua::ai::server::register_library(Interface &lua)
{
	auto modAi = luabind::module_(lua.GetState(), "ai");
	modAi[(luabind::def("create_schedule", create_schedule), luabind::def("register_faction", register_faction), luabind::def("find_faction_by_name", find_faction_by_name), luabind::def("get_factions", get_factions),
	  luabind::def("register_task", static_cast<uint32_t (*)(lua::State *, const LuaClassObject &, pragma::ai::BehaviorNode::Type, pragma::ai::SelectorType)>(register_task)),
	  luabind::def("register_task", static_cast<uint32_t (*)(lua::State *, const LuaClassObject &, pragma::ai::BehaviorNode::Type)>(register_task)), luabind::def("register_task", static_cast<uint32_t (*)(lua::State *, const LuaClassObject &)>(register_task)))];
	ai::register_library(lua);

	auto *l = lua.GetState();
	RegisterLibraryEnums(l, "ai",
	  {{"DISPOSITION_HATE", pragma::math::to_integral(DISPOSITION::HATE)}, {"DISPOSITION_FEAR", pragma::math::to_integral(DISPOSITION::FEAR)}, {"DISPOSITION_NEUTRAL", pragma::math::to_integral(DISPOSITION::NEUTRAL)}, {"DISPOSITION_LIKE", pragma::math::to_integral(DISPOSITION::LIKE)},

	    {"NPC_STATE_NONE", pragma::math::to_integral(NPCSTATE::NONE)}, {"NPC_STATE_IDLE", pragma::math::to_integral(NPCSTATE::IDLE)}, {"NPC_STATE_ALERT", pragma::math::to_integral(NPCSTATE::ALERT)}, {"NPC_STATE_COMBAT", pragma::math::to_integral(NPCSTATE::COMBAT)},
	    {"NPC_STATE_SCRIPT", pragma::math::to_integral(NPCSTATE::SCRIPT)},

	    {"TASK_MOVE_TO_TARGET", pragma::math::to_integral(pragma::ai::Task::MoveToTarget)}, {"TASK_PLAY_ANIMATION", pragma::math::to_integral(pragma::ai::Task::PlayAnimation)}, {"TASK_PLAY_ACTIVITY", pragma::math::to_integral(pragma::ai::Task::PlayActivity)},
	    {"TASK_PLAY_LAYERED_ANIMATION", pragma::math::to_integral(pragma::ai::Task::PlayLayeredAnimation)}, {"TASK_PLAY_LAYERED_ACTIVITY", pragma::math::to_integral(pragma::ai::Task::PlayLayeredActivity)}, {"TASK_MOVE_RANDOM", pragma::math::to_integral(pragma::ai::Task::MoveRandom)},
	    {"TASK_PLAY_SOUND", pragma::math::to_integral(pragma::ai::Task::PlaySound)}, {"TASK_DEBUG_PRINT", pragma::math::to_integral(pragma::ai::Task::DebugPrint)}, {"TASK_DEBUG_DRAW_TEXT", pragma::math::to_integral(pragma::ai::Task::DebugDrawText)},
	    {"TASK_DECORATOR", pragma::math::to_integral(pragma::ai::Task::Decorator)}, {"TASK_WAIT", pragma::math::to_integral(pragma::ai::Task::Wait)}, {"TASK_TURN_TO_TARGET", pragma::math::to_integral(pragma::ai::Task::TurnToTarget)},
	    {"TASK_RANDOM", pragma::math::to_integral(pragma::ai::Task::Random)}, {"TASK_LOOK_AT_TARGET", pragma::math::to_integral(pragma::ai::Task::LookAtTarget)}, {"TASK_EVENT", pragma::math::to_integral(pragma::ai::Task::Event)},

	    {"MOVE_STATE_TARGET_UNREACHABLE", pragma::math::to_integral(pragma::SAIComponent::MoveResult::TargetUnreachable)}, {"MOVE_STATE_TARGET_REACHED", pragma::math::to_integral(pragma::SAIComponent::MoveResult::TargetReached)},
	    {"MOVE_STATE_WAITING_FOR_PATH", pragma::math::to_integral(pragma::SAIComponent::MoveResult::WaitingForPath)}, {"MOVE_STATE_MOVING_TO_TARGET", pragma::math::to_integral(pragma::SAIComponent::MoveResult::MovingToTarget)}});

	AISquad::register_class(l, modAi);
	AIMemory::register_class(l, modAi);
	Faction::register_class(l, modAi);
	AISchedule::register_class(l, modAi);
	AIBehaviorNode::register_class(l, modAi);

	auto cdMoveToTarget = luabind::class_<TaskWrapperMoveToTarget, TaskWrapper>("TaskMoveToTarget");
	cdMoveToTarget.add_static_constant("PARAMETER_TARGET", pragma::math::to_integral(pragma::ai::TaskMoveToTarget::Parameter::Target));
	cdMoveToTarget.add_static_constant("PARAMETER_DISTANCE", pragma::math::to_integral(pragma::ai::TaskMoveToTarget::Parameter::Distance));
	cdMoveToTarget.add_static_constant("PARAMETER_MOVE_ACTIVITY", pragma::math::to_integral(pragma::ai::TaskMoveToTarget::Parameter::MoveActivity));
	cdMoveToTarget.def("SetMoveTarget", static_cast<void (*)(lua::State *, TaskWrapperMoveToTarget &, const Vector3 &)>([](lua::State *l, TaskWrapperMoveToTarget &task, const Vector3 &target) { task->SetTarget(target); }));
	cdMoveToTarget.def("SetMoveTarget", static_cast<void (*)(lua::State *, TaskWrapperMoveToTarget &, pragma::ecs::BaseEntity &)>([](lua::State *l, TaskWrapperMoveToTarget &task, pragma::ecs::BaseEntity &ent) { task->SetTarget(ent.GetHandle()); }));
	cdMoveToTarget.def("SetMoveDistance", static_cast<void (*)(lua::State *, TaskWrapperMoveToTarget &, float)>([](lua::State *l, TaskWrapperMoveToTarget &task, float dist) { task->SetMoveDistance(dist); }));
	cdMoveToTarget.def("SetMoveActivity",
	  static_cast<void (*)(lua::State *, TaskWrapperMoveToTarget &, std::underlying_type_t<pragma::Activity>)>([](lua::State *l, TaskWrapperMoveToTarget &task, std::underlying_type_t<pragma::Activity> activity) { task->SetMoveActivity(static_cast<pragma::Activity>(activity)); }));
	modAi[cdMoveToTarget];

	auto cdMoveRandom = luabind::class_<TaskWrapperMoveRandom, TaskWrapper>("TaskMoveRandom");
	cdMoveRandom.add_static_constant("PARAMETER_DISTANCE", pragma::math::to_integral(pragma::ai::TaskMoveRandom::Parameter::Distance));
	cdMoveRandom.add_static_constant("PARAMETER_MOVE_ACTIVITY", pragma::math::to_integral(pragma::ai::TaskMoveRandom::Parameter::MoveActivity));
	cdMoveRandom.def("SetMoveDistance", static_cast<void (*)(lua::State *, TaskWrapperMoveRandom &, float)>([](lua::State *l, TaskWrapperMoveRandom &task, float dist) { task->SetMoveDistance(dist); }));
	cdMoveRandom.def("SetMoveActivity",
	  static_cast<void (*)(lua::State *, TaskWrapperMoveRandom &, std::underlying_type_t<pragma::Activity>)>([](lua::State *l, TaskWrapperMoveRandom &task, std::underlying_type_t<pragma::Activity> activity) { task->SetMoveActivity(static_cast<pragma::Activity>(activity)); }));
	modAi[cdMoveRandom];

	auto cdLookAtTarget = luabind::class_<TaskWrapperLookAtTarget, TaskWrapper>("TaskLookAtTarget");
	cdLookAtTarget.add_static_constant("PARAMETER_LOOK_DURATION", pragma::math::to_integral(pragma::ai::TaskLookAtTarget::Parameter::LookDuration));
	cdLookAtTarget.def("SetLookDuration", static_cast<void (*)(lua::State *, TaskWrapperLookAtTarget &, float)>([](lua::State *l, TaskWrapperLookAtTarget &task, float duration) { task->SetLookDuration(duration); }));
	modAi[cdLookAtTarget];

	auto cdPlayAnimation = luabind::class_<TaskWrapperPlayAnimation, TaskWrapper>("TaskPlayAnimation");
	cdPlayAnimation.add_static_constant("PARAMETER_ANIMATION", pragma::math::to_integral(pragma::ai::TaskPlayAnimation::Parameter::Animation));
	cdPlayAnimation.add_static_constant("PARAMETER_FACE_TARGET", pragma::math::to_integral(pragma::ai::TaskPlayAnimation::Parameter::FaceTarget));
	cdPlayAnimation.def("SetAnimation", static_cast<void (*)(lua::State *, TaskWrapperPlayAnimation &, int32_t)>([](lua::State *l, TaskWrapperPlayAnimation &task, int32_t animation) { task->SetAnimation(animation); }));
	cdPlayAnimation.def("SetAnimation", static_cast<void (*)(lua::State *, TaskWrapperPlayAnimation &, const std::string &)>([](lua::State *l, TaskWrapperPlayAnimation &task, const std::string &animation) { task->SetAnimation(animation); }));
	cdPlayAnimation.def("SetFaceTarget", static_cast<void (*)(lua::State *, TaskWrapperPlayAnimation &, const Vector3 &)>([](lua::State *l, TaskWrapperPlayAnimation &task, const Vector3 &target) { task->SetFaceTarget(target); }));
	cdPlayAnimation.def("SetFaceTarget", static_cast<void (*)(lua::State *, TaskWrapperPlayAnimation &, pragma::ecs::BaseEntity &)>([](lua::State *l, TaskWrapperPlayAnimation &task, pragma::ecs::BaseEntity &target) { task->SetFaceTarget(target); }));
	cdPlayAnimation.def("SetFacePrimaryTarget", static_cast<void (*)(lua::State *, TaskWrapperPlayAnimation &)>([](lua::State *l, TaskWrapperPlayAnimation &task) { task->SetFacePrimaryTarget(); }));
	modAi[cdPlayAnimation];

	auto cdPlayActivity = luabind::class_<TaskWrapperPlayActivity, TaskWrapper>("TaskPlayActivity");
	cdPlayActivity.add_static_constant("PARAMETER_ACTIVITY", pragma::math::to_integral(pragma::ai::TaskPlayActivity::Parameter::Activity));
	cdPlayActivity.add_static_constant("PARAMETER_FACE_TARGET", pragma::math::to_integral(pragma::ai::TaskPlayActivity::Parameter::FaceTarget));
	cdPlayActivity.def("SetActivity",
	  static_cast<void (*)(lua::State *, TaskWrapperPlayActivity &, std::underlying_type_t<pragma::Activity>)>([](lua::State *l, TaskWrapperPlayActivity &task, std::underlying_type_t<pragma::Activity> activity) { task->SetActivity(static_cast<pragma::Activity>(activity)); }));
	cdPlayActivity.def("SetFaceTarget", static_cast<void (*)(lua::State *, TaskWrapperPlayActivity &, const Vector3 &)>([](lua::State *l, TaskWrapperPlayActivity &task, const Vector3 &target) { task->SetFaceTarget(target); }));
	cdPlayActivity.def("SetFaceTarget", static_cast<void (*)(lua::State *, TaskWrapperPlayActivity &, pragma::ecs::BaseEntity &)>([](lua::State *l, TaskWrapperPlayActivity &task, pragma::ecs::BaseEntity &target) { task->SetFaceTarget(target); }));
	cdPlayActivity.def("SetFacePrimaryTarget", static_cast<void (*)(lua::State *, TaskWrapperPlayActivity &)>([](lua::State *l, TaskWrapperPlayActivity &task) { task->SetFacePrimaryTarget(); }));
	modAi[cdPlayActivity];

	auto cdPlayLayeredAnimation = luabind::class_<TaskWrapperPlayLayeredAnimation, TaskWrapper>("TaskPlayLayeredAnimation");
	cdPlayLayeredAnimation.add_static_constant("PARAMETER_ANIMATION", pragma::math::to_integral(pragma::ai::TaskPlayLayeredAnimation::Parameter::Animation));
	cdPlayLayeredAnimation.add_static_constant("PARAMETER_ANIMATION_SLOT", pragma::math::to_integral(pragma::ai::TaskPlayLayeredAnimation::Parameter::AnimationSlot));
	cdPlayLayeredAnimation.def("SetAnimation", static_cast<void (*)(lua::State *, TaskWrapperPlayLayeredAnimation &, int32_t)>([](lua::State *l, TaskWrapperPlayLayeredAnimation &task, int32_t animation) { task->SetAnimation(animation); }));
	cdPlayLayeredAnimation.def("SetAnimation", static_cast<void (*)(lua::State *, TaskWrapperPlayLayeredAnimation &, const std::string &)>([](lua::State *l, TaskWrapperPlayLayeredAnimation &task, const std::string &animation) { task->SetAnimation(animation); }));
	cdPlayLayeredAnimation.def("SetAnimationSlot", static_cast<void (*)(lua::State *, TaskWrapperPlayLayeredAnimation &, int32_t)>([](lua::State *l, TaskWrapperPlayLayeredAnimation &task, int32_t animationSlot) { task->SetAnimationSlot(animationSlot); }));
	modAi[cdPlayLayeredAnimation];

	auto cdPlayLayeredActivity = luabind::class_<TaskWrapperPlayLayeredActivity, TaskWrapper>("TaskPlayLayeredActivity");
	cdPlayLayeredActivity.add_static_constant("PARAMETER_ACTIVITY", pragma::math::to_integral(pragma::ai::TaskPlayLayeredActivity::Parameter::Activity));
	cdPlayLayeredActivity.add_static_constant("PARAMETER_ANIMATION_SLOT", pragma::math::to_integral(pragma::ai::TaskPlayLayeredActivity::Parameter::AnimationSlot));
	cdPlayLayeredActivity.def("SetActivity", static_cast<void (*)(lua::State *, TaskWrapperPlayLayeredActivity &, std::underlying_type_t<pragma::Activity>)>([](lua::State *l, TaskWrapperPlayLayeredActivity &task, std::underlying_type_t<pragma::Activity> activity) {
		task->SetActivity(static_cast<pragma::Activity>(activity));
	}));
	cdPlayLayeredActivity.def("SetAnimationSlot", static_cast<void (*)(lua::State *, TaskWrapperPlayLayeredActivity &, int32_t)>([](lua::State *l, TaskWrapperPlayLayeredActivity &task, int32_t animationSlot) { task->SetAnimationSlot(animationSlot); }));
	modAi[cdPlayLayeredActivity];

	auto cdPlaySound = luabind::class_<TaskWrapperPlaySound, TaskWrapper>("TaskPlaySound");
	cdPlaySound.add_static_constant("PARAMETER_SOUND_NAME", pragma::math::to_integral(pragma::ai::TaskPlaySound::Parameter::SoundName));
	cdPlaySound.add_static_constant("PARAMETER_GAIN", pragma::math::to_integral(pragma::ai::TaskPlaySound::Parameter::Gain));
	cdPlaySound.add_static_constant("PARAMETER_PITCH", pragma::math::to_integral(pragma::ai::TaskPlaySound::Parameter::Pitch));
	cdPlaySound.def("SetSoundName", static_cast<void (*)(lua::State *, TaskWrapperPlaySound &, const std::string &)>([](lua::State *l, TaskWrapperPlaySound &task, const std::string &sndName) { task->SetSoundName(sndName); }));
	cdPlaySound.def("SetGain", static_cast<void (*)(lua::State *, TaskWrapperPlaySound &, float)>([](lua::State *l, TaskWrapperPlaySound &task, float gain) { task->SetGain(gain); }));
	cdPlaySound.def("SetPitch", static_cast<void (*)(lua::State *, TaskWrapperPlaySound &, float)>([](lua::State *l, TaskWrapperPlaySound &task, float pitch) { task->SetPitch(pitch); }));
	modAi[cdPlaySound];

	auto cdRandom = luabind::class_<TaskWrapperRandom, TaskWrapper>("TaskRandom");
	cdRandom.add_static_constant("PARAMETER_CHANCE", pragma::math::to_integral(pragma::ai::TaskRandom::Parameter::Chance));
	cdRandom.def("SetChance", static_cast<void (*)(lua::State *, TaskWrapperRandom &, float)>([](lua::State *l, TaskWrapperRandom &task, float chance) { task->SetChance(chance); }));
	modAi[cdRandom];

	auto cdDebugPrint = luabind::class_<TaskWrapperDebugPrint, TaskWrapper>("TaskDebugPrint");
	cdDebugPrint.add_static_constant("PARAMETER_MESSAGE", pragma::math::to_integral(pragma::ai::TaskDebugPrint::Parameter::Message));
	cdDebugPrint.def("SetMessage", static_cast<void (*)(lua::State *, TaskWrapperDebugPrint &, const std::string &)>([](lua::State *l, TaskWrapperDebugPrint &task, const std::string &msg) { task->SetMessage(msg); }));
	modAi[cdDebugPrint];

	auto cdDebugDrawText = luabind::class_<TaskWrapperDebugDrawText, TaskWrapper>("TaskDebugDrawText");
	cdDebugDrawText.add_static_constant("PARAMETER_MESSAGE", pragma::math::to_integral(pragma::ai::TaskDebugDrawText::Parameter::Message));
	cdDebugDrawText.def("SetMessage", static_cast<void (*)(lua::State *, TaskWrapperDebugDrawText &, const std::string &)>([](lua::State *l, TaskWrapperDebugDrawText &task, const std::string &msg) { task->SetMessage(msg); }));
	modAi[cdDebugDrawText];

	auto cdDecorator = luabind::class_<TaskWrapperDecorator, TaskWrapper>("TaskDecorator");
	cdDecorator.add_static_constant("PARAMETER_DECORATOR_TYPE", pragma::math::to_integral(pragma::ai::TaskDecorator::Parameter::DecoratorType));
	cdDecorator.add_static_constant("PARAMETER_LIMIT", pragma::math::to_integral(pragma::ai::TaskDecorator::Parameter::Limit));
	cdDecorator.def("SetDecoratorType",
	  static_cast<void (*)(lua::State *, TaskWrapperDecorator &, std::underlying_type_t<pragma::ai::TaskDecorator::DecoratorType>)>(
	    [](lua::State *l, TaskWrapperDecorator &task, std::underlying_type_t<pragma::ai::TaskDecorator::DecoratorType> decoratorType) { task->SetDecoratorType(static_cast<pragma::ai::TaskDecorator::DecoratorType>(decoratorType)); }));
	cdDecorator.def("SetLimit", static_cast<void (*)(lua::State *, TaskWrapperDecorator &, uint32_t)>([](lua::State *l, TaskWrapperDecorator &task, uint32_t limit) { task->SetLimit(limit); }));
	modAi[cdDecorator];

	auto cdEvent = luabind::class_<TaskWrapperEvent, TaskWrapper>("TaskEvent");
	cdEvent.add_static_constant("PARAMETER_EVENT_ID", pragma::math::to_integral(pragma::ai::TaskEvent::Parameter::EventId));
	cdEvent.add_static_constant("PARAMETER_EVENT_ARGS_START", pragma::math::to_integral(pragma::ai::TaskEvent::Parameter::EventArgStart));
	cdEvent.def("SetEventId", static_cast<void (*)(lua::State *, TaskWrapperEvent &, std::underlying_type_t<pragma::AnimationEvent::Type>)>([](lua::State *l, TaskWrapperEvent &task, std::underlying_type_t<pragma::AnimationEvent::Type> eventId) {
		task->SetEventId(static_cast<pragma::AnimationEvent::Type>(eventId));
	}));
	cdEvent.def("SetEventArgument", static_cast<void (*)(lua::State *, TaskWrapperEvent &, uint32_t, const std::string &)>([](lua::State *l, TaskWrapperEvent &task, uint32_t argIdx, const std::string &arg) { task->SetEventArgument(argIdx, arg); }));
	modAi[cdEvent];

	auto cdWait = luabind::class_<TaskWrapperWait, TaskWrapper>("TaskWait");
	cdWait.add_static_constant("PARAMETER_MIN_WAIT_TIME", pragma::math::to_integral(pragma::ai::TaskWait::Parameter::MinWaitTime));
	cdWait.add_static_constant("PARAMETER_MAX_WAIT_TIME", pragma::math::to_integral(pragma::ai::TaskWait::Parameter::MaxWaitTime));
	cdWait.def("SetMinWaitTime", static_cast<void (*)(lua::State *, TaskWrapperWait &, float)>([](lua::State *l, TaskWrapperWait &task, float t) { task->SetMinWaitTime(t); }));
	cdWait.def("SetMaxWaitTime", static_cast<void (*)(lua::State *, TaskWrapperWait &, float)>([](lua::State *l, TaskWrapperWait &task, float t) { task->SetMaxWaitTime(t); }));
	cdWait.def("SetWaitTime", static_cast<void (*)(lua::State *, TaskWrapperWait &, float, float)>([](lua::State *l, TaskWrapperWait &task, float tMin, float tMax) { task->SetWaitTime(tMin, tMax); }));
	cdWait.def("SetWaitTime", static_cast<void (*)(lua::State *, TaskWrapperWait &, float)>([](lua::State *l, TaskWrapperWait &task, float t) { task->SetWaitTime(t, t); }));
	modAi[cdWait];
}

std::shared_ptr<pragma::ai::BehaviorNode> Lua::ai::server::create_lua_task(lua::State *l, luabind::object oClass, pragma::ai::BehaviorNode::Type taskType, pragma::ai::SelectorType selectorType)
{
	luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
	try {
#endif
		r = oClass(pragma::math::to_integral(taskType), pragma::math::to_integral(selectorType));
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error &) {
		HandleLuaError(l);
		return nullptr;
	}
#endif
	if(!r) {
		Con::CWAR << Con::PREFIX_SERVER << "Unable to create lua AI Task!" << Con::endl;
		return nullptr;
	}
	auto *ptr = luabind::object_cast_nothrow<AILuaBehaviorNodeWrapper *>(r, static_cast<AILuaBehaviorNodeWrapper *>(nullptr));
	if(ptr) {
		auto &luaNode = static_cast<AILuaBehaviorNode &>(ptr->GetTask());
		luaNode.SetLuaObject(r); //Lua::CreateWeakReference(r)); // TODO: This works if the task is attached to another task, but what if it's not? -> DANGLING REFERENCE!!!
		luaNode.SetLuaClass(oClass);
		return std::static_pointer_cast<pragma::ai::BehaviorNode>(luaNode.shared_from_this());
	}
	Con::CWAR << Con::PREFIX_SERVER << "Unable to create lua AI Task: Lua class is not derived from valid AI Task base!" << Con::endl;
	return nullptr;
}

std::shared_ptr<pragma::ai::Schedule> Lua::ai::create_schedule() { return pragma::ai::Schedule::Create(); }

std::shared_ptr<Faction> Lua::ai::register_faction(const std::string &name)
{
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	return factionManager.RegisterFaction(name);
}
LuaTableObject Lua::ai::get_factions(lua::State *l)
{
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	auto &factions = factionManager.GetFactions();
	return vector_to_table(l, factions);
}
LuaFactionObject Lua::ai::find_faction_by_name(lua::State *l, const std::string &name)
{
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	auto faction = factionManager.FindFactionByName(name);
	if(faction == nullptr)
		return {};
	return luabind::object {l, faction};
}

uint32_t Lua::ai::register_task(lua::State *l, const LuaClassObject &taskClass, pragma::ai::BehaviorNode::Type taskType, pragma::ai::SelectorType selectorType)
{
	CheckUserData(l, 1);
	auto &taskManager = pragma::SGame::Get()->GetAITaskManager();
	return taskManager.RegisterTask(std::bind(server::create_lua_task, l, taskClass, taskType, selectorType));
}
uint32_t Lua::ai::register_task(lua::State *l, const LuaClassObject &taskClass, pragma::ai::BehaviorNode::Type taskType) { return register_task(l, taskClass, taskType, pragma::ai::SelectorType::Sequential); }
uint32_t Lua::ai::register_task(lua::State *l, const LuaClassObject &taskClass) { return register_task(l, taskClass, pragma::ai::BehaviorNode::Type::Sequence); }
