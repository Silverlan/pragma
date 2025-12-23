// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.server;
import :scripting.lua.classes.ai_behavior;

import :ai;
import :entities.components;
import :game;
import :scripting.lua.libraries.ai;

namespace Lua {
	namespace AIBehaviorNode {
		static void GetType(lua::State *l, ai::TaskWrapper &task);
		static void GetSelectorType(lua::State *l, ai::TaskWrapper &task);
		static void IsActive(lua::State *l, ai::TaskWrapper &task);
		static void GetNode(lua::State *l, ai::TaskWrapper &task, uint32_t nodeId);
		static void GetNodes(lua::State *l, ai::TaskWrapper &task);
		static void SetScheduleParameter(lua::State *l, ai::TaskWrapper &task, uint8_t taskParamId, uint8_t scheduleParamId);
		static void HasParameter(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void LinkParameter(lua::State *l, ai::TaskWrapper &task, uint8_t paramIdx, ai::TaskWrapper &taskOther, uint8_t paramIdxOther);
		static void LinkParameter(lua::State *l, ai::TaskWrapper &task, uint8_t paramIdx, ai::TaskWrapper &taskOther);
		static void GetParameterBool(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void GetParameterInt(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void GetParameterFloat(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void GetParameterString(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void GetParameterEntity(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void GetParameterVector(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void GetParameterQuaternion(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void GetParameterEulerAngles(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);

		static void GetParameterBool(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, bool def);
		static void GetParameterInt(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, int32_t def);
		static void GetParameterFloat(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, float def);
		static void GetParameterString(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const std::string &def);
		static void GetParameterEntity(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, pragma::ecs::BaseEntity *def);
		static void GetParameterVector(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const Vector3 &def);
		static void GetParameterQuaternion(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const Quat &def);
		static void GetParameterEulerAngles(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const EulerAngles &def);

		static void GetParameterType(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx);
		static void SetDebugName(lua::State *l, ai::TaskWrapper &task, const std::string &name);
		static void GetDebugName(lua::State *l, ai::TaskWrapper &task);

		static void SetParameterBool(lua::State *l, ai::TaskWrapper &task, uint8_t idx, bool b);
		static void SetParameterInt(lua::State *l, ai::TaskWrapper &task, uint8_t idx, int32_t i);
		static void SetParameterFloat(lua::State *l, ai::TaskWrapper &task, uint8_t idx, float f);
		static void SetParameterString(lua::State *l, ai::TaskWrapper &task, uint8_t idx, const std::string &s);
		static void SetParameterEntity(lua::State *l, ai::TaskWrapper &task, uint8_t idx, pragma::ecs::BaseEntity &ent);
		static void SetParameterVector(lua::State *l, ai::TaskWrapper &task, uint8_t idx, const Vector3 &v);
		static void SetParameterQuaternion(lua::State *l, ai::TaskWrapper &task, uint8_t idx, const Quat &rot);
		static void SetParameterEulerAngles(lua::State *l, ai::TaskWrapper &task, uint8_t idx, const EulerAngles &ang);

		static void CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o, pragma::ai::BehaviorNode::Type *taskType, pragma::ai::SelectorType *selectorType, int32_t aiTaskTypeParamOffset = 2);
		static void CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o, uint32_t taskType, uint32_t selectorType);
		static void CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o, uint32_t taskType);
		static void CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o);

		static void CreateDecoratedTask(lua::State *l, ai::TaskWrapper &task, uint32_t decoratorType, luabind::object o, uint32_t taskType, uint32_t selectorType);
		static void CreateDecoratedTask(lua::State *l, ai::TaskWrapper &task, uint32_t decoratorType, luabind::object o, uint32_t taskType);
		static void CreateDecoratedTask(lua::State *l, ai::TaskWrapper &task, uint32_t decoratorType, luabind::object o);
	};
};

Lua::ai::TaskWrapper::TaskWrapper(pragma::ai::BehaviorNode &task) : TaskWrapper {task.shared_from_this()} {}
Lua::ai::TaskWrapper::TaskWrapper(const std::shared_ptr<pragma::ai::BehaviorNode> &task) : m_task {task} {}

pragma::ai::BehaviorNode &Lua::ai::TaskWrapper::GetTask() { return *m_task; }
const pragma::ai::BehaviorNode &Lua::ai::TaskWrapper::GetTask() const { return *m_task; }

const pragma::ai::BehaviorNode &Lua::ai::TaskWrapper::operator*() const { return GetTask(); }
pragma::ai::BehaviorNode &Lua::ai::TaskWrapper::operator*() { return GetTask(); }

const pragma::ai::BehaviorNode *Lua::ai::TaskWrapper::operator->() const { return m_task.get(); }
pragma::ai::BehaviorNode *Lua::ai::TaskWrapper::operator->() { return m_task.get(); }

void Lua::ai::push_task(lua::State *l, pragma::ai::BehaviorNode &task)
{
	if(typeid(task) == typeid(pragma::ai::TaskMoveToTarget))
		Lua::Push<TaskWrapperMoveToTarget>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskMoveRandom))
		Lua::Push<TaskWrapperMoveRandom>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskLookAtTarget))
		Lua::Push<TaskWrapperLookAtTarget>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskPlayAnimation))
		Lua::Push<TaskWrapperPlayAnimation>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskPlayActivity))
		Lua::Push<TaskWrapperPlayActivity>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskPlayLayeredAnimation))
		Lua::Push<TaskWrapperPlayLayeredAnimation>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskPlayLayeredActivity))
		Lua::Push<TaskWrapperPlayLayeredActivity>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskPlaySound))
		Lua::Push<TaskWrapperPlaySound>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskRandom))
		Lua::Push<TaskWrapperRandom>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskDebugPrint))
		Lua::Push<TaskWrapperDebugPrint>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskDebugDrawText))
		Lua::Push<TaskWrapperDebugDrawText>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskDecorator))
		Lua::Push<TaskWrapperDecorator>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskEvent))
		Lua::Push<TaskWrapperEvent>(l, {task});
	else if(typeid(task) == typeid(pragma::ai::TaskWait))
		Lua::Push<TaskWrapperWait>(l, {task});
	else if(typeid(task) == typeid(AILuaBehaviorNode))
		static_cast<AILuaBehaviorNode &>(task).GetLuaObject().push(l);
	else
		Lua::Push<TaskWrapper>(l, {task});
}

//////////

AILuaBehaviorNode::AILuaBehaviorNode(Type type, pragma::ai::SelectorType selectorType) : BehaviorNode(type, selectorType), LuaObjectBase() {}
AILuaBehaviorNode::~AILuaBehaviorNode() {}
void AILuaBehaviorNode::Print(const pragma::ai::Schedule *sched, std::ostream &o) const { o << ToString(); }
void AILuaBehaviorNode::Clear()
{
	ClearLuaObject();
	BehaviorNode::Clear();
}
void AILuaBehaviorNode::OnTaskComplete(const pragma::ai::Schedule *sched, uint32_t taskId, Result result)
{
	BehaviorNode::OnTaskComplete(sched, taskId, result);
	CallLuaMember<void, std::shared_ptr<pragma::ai::Schedule>, uint32_t, std::underlying_type_t<decltype(result)>>("OnTaskComplete", const_cast<pragma::ai::Schedule *>(sched)->shared_from_this(), taskId, pragma::math::to_integral(result));
}
void AILuaBehaviorNode::SetLuaClass(const luabind::object &o) { m_luaClass = o; }
std::shared_ptr<pragma::ai::BehaviorNode> AILuaBehaviorNode::Copy() const { return Lua::ai::server::create_lua_task(m_luaClass.interpreter(), m_luaClass, m_type, m_selector->GetType()); }
void AILuaBehaviorNode::Stop()
{
	BehaviorNode::Stop();
	CallLuaMember<void>("Stop");
}
pragma::ai::BehaviorNode::Result AILuaBehaviorNode::Think(const pragma::ai::Schedule *sched, pragma::BaseAIComponent &ent)
{
	auto r = BehaviorNode::Think(sched, ent);
	if(r == Result::Pending)
		return r;
	auto luaResult = pragma::math::to_integral(Result::Succeeded);
	if(CallLuaMember<uint32_t, std::shared_ptr<pragma::ai::Schedule>, luabind::object, std::underlying_type_t<decltype(r)>>("Think", &luaResult, const_cast<pragma::ai::Schedule *>(sched)->shared_from_this(), ent.GetLuaObject(), pragma::math::to_integral(r)) == CallbackReturnType::HasReturnValue)
		r = static_cast<Result>(luaResult);
	return r;
}
pragma::ai::BehaviorNode::Result AILuaBehaviorNode::Start(const pragma::ai::Schedule *sched, pragma::BaseAIComponent &ent)
{
	auto r = BehaviorNode::Start(sched, ent);
	auto luaResult = pragma::math::to_integral(Result::Succeeded);
	if(CallLuaMember<uint32_t, std::shared_ptr<pragma::ai::Schedule>, luabind::object>("Start", &luaResult, const_cast<pragma::ai::Schedule *>(sched)->shared_from_this(), ent.GetLuaObject()) == CallbackReturnType::HasReturnValue)
		r = static_cast<Result>(luaResult);
	return r;
}
void AILuaBehaviorNode::SetScheduleParameter(uint8_t taskParamId, uint8_t scheduleParamId)
{
	BehaviorNode::SetScheduleParameter(taskParamId, scheduleParamId);
	CallLuaMember<void, uint8_t, uint8_t>("OnSetScheduleParameter", taskParamId, scheduleParamId);
}

AILuaBehaviorNodeWrapper::AILuaBehaviorNodeWrapper(uint32_t nodeType, uint32_t selectorType) : BaseBehaviorTask(pragma::util::make_shared<AILuaBehaviorNode>(static_cast<pragma::ai::BehaviorNode::Type>(nodeType), static_cast<pragma::ai::SelectorType>(selectorType))), wrap_base() {}

AILuaBehaviorNodeWrapper::AILuaBehaviorNodeWrapper(uint32_t nodeType) : AILuaBehaviorNodeWrapper(nodeType, pragma::math::to_integral(pragma::ai::SelectorType::Sequential)) {}

AILuaBehaviorNodeWrapper::AILuaBehaviorNodeWrapper() : AILuaBehaviorNodeWrapper(pragma::math::to_integral(pragma::ai::BehaviorNode::Type::Sequence)) {}

uint32_t AILuaBehaviorNodeWrapper::Start(std::shared_ptr<pragma::ai::Schedule> &, pragma::BaseAIComponent &) { return pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Succeeded); }
uint32_t AILuaBehaviorNodeWrapper::default_Start(lua::State *, AILuaBehaviorNodeWrapper &, std::shared_ptr<pragma::ai::Schedule> &, pragma::BaseAIComponent &) { return pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Succeeded); }

void AILuaBehaviorNodeWrapper::Stop() {}
void AILuaBehaviorNodeWrapper::default_Stop(lua::State *, AILuaBehaviorNodeWrapper &) {}

uint32_t AILuaBehaviorNodeWrapper::Think(std::shared_ptr<pragma::ai::Schedule> &, pragma::BaseAIComponent &, std::underlying_type_t<pragma::ai::BehaviorNode::Result>) { return pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Succeeded); }
uint32_t AILuaBehaviorNodeWrapper::default_Think(lua::State *, AILuaBehaviorNodeWrapper &, std::shared_ptr<pragma::ai::Schedule> &, pragma::BaseAIComponent &, std::underlying_type_t<pragma::ai::BehaviorNode::Result>)
{
	return pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Succeeded);
}

void AILuaBehaviorNodeWrapper::OnTaskComplete(std::shared_ptr<pragma::ai::Schedule> &schedule, uint32_t taskId, std::underlying_type_t<pragma::ai::BehaviorNode::Result> result) {}
void AILuaBehaviorNodeWrapper::default_OnTaskComplete(lua::State *l, AILuaBehaviorNodeWrapper &wrapper, std::shared_ptr<pragma::ai::Schedule> &schedule, uint32_t taskId, std::underlying_type_t<pragma::ai::BehaviorNode::Result> result) {}

void AILuaBehaviorNodeWrapper::OnSetScheduleParameter(uint8_t, uint8_t) {}
void AILuaBehaviorNodeWrapper::default_OnSetScheduleParameter(lua::State *, AILuaBehaviorNodeWrapper &, uint8_t, uint8_t) {}

inline std::ostream &operator<<(std::ostream &s, const AILuaBehaviorNodeWrapper &task) { return s << typeid(AILuaBehaviorNode).name(); }
inline std::ostream &operator<<(std::ostream &s, const Lua::ai::TaskWrapper &task)
{
	std::stringstream ss {};
	task->DebugPrint(nullptr, ss);
	return s << ss.str();
}

#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(Lua::ai, TaskWrapper);
#endif

void Lua::AIBehaviorNode::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<ai::TaskWrapper>("BehaviorTask");
	classDef.def("GetType", &GetType);
	classDef.def("GetSelectorType", &GetSelectorType);
	classDef.def("IsActive", &IsActive);
	classDef.def("GetTask", &GetNode);
	classDef.def("GetTasks", &GetNodes);
	classDef.def("SetScheduleParameter", &SetScheduleParameter);
	classDef.def("LinkParameter", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, ai::TaskWrapper &, uint8_t)>(&LinkParameter));
	classDef.def("LinkParameter", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, ai::TaskWrapper &)>(&LinkParameter));
	classDef.def("HasParameter", &HasParameter);
	classDef.def(luabind::tostring(luabind::self));

	classDef.def("GetParameterBool", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, bool)>(&GetParameterBool));
	classDef.def("GetParameterBool", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterBool));
	classDef.def("GetParameterInt", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, int32_t)>(&GetParameterInt));
	classDef.def("GetParameterInt", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterInt));
	classDef.def("GetParameterFloat", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, float)>(&GetParameterFloat));
	classDef.def("GetParameterFloat", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterFloat));
	classDef.def("GetParameterString", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, const std::string &)>(&GetParameterString));
	classDef.def("GetParameterString", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterString));
	classDef.def("GetParameterEntity", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, pragma::ecs::BaseEntity *)>(&GetParameterEntity));
	classDef.def("GetParameterEntity", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterEntity));
	classDef.def("GetParameterVector", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, const Vector3 &)>(&GetParameterVector));
	classDef.def("GetParameterVector", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterVector));
	classDef.def("GetParameterQuaternion", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, const Quat &)>(&GetParameterQuaternion));
	classDef.def("GetParameterQuaternion", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterQuaternion));
	classDef.def("GetParameterEulerAngles", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t, const EulerAngles &)>(&GetParameterEulerAngles));
	classDef.def("GetParameterEulerAngles", static_cast<void (*)(lua::State *, ai::TaskWrapper &, std::shared_ptr<pragma::ai::Schedule> &, uint8_t)>(&GetParameterEulerAngles));

	classDef.def("GetParameterType", &GetParameterType);
	classDef.def("SetDebugName", &SetDebugName);
	classDef.def("GetDebugName", &GetDebugName);

	classDef.def("SetParameterBool", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, bool)>(&SetParameterBool));
	classDef.def("SetParameterInt", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, int32_t)>(&SetParameterInt));
	classDef.def("SetParameterFloat", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, float)>(&SetParameterFloat));
	classDef.def("SetParameterString", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, const std::string &)>(&SetParameterString));
	classDef.def("SetParameterEntity", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, pragma::ecs::BaseEntity &)>(&SetParameterEntity));
	classDef.def("SetParameterVector", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, const Vector3 &)>(&SetParameterVector));
	classDef.def("SetParameterQuaternion", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, const Quat &)>(&SetParameterQuaternion));
	classDef.def("SetParameterEulerAngles", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint8_t, const EulerAngles &)>(&SetParameterEulerAngles));

	classDef.def("CreateTask", static_cast<void (*)(lua::State *, ai::TaskWrapper &, luabind::object, uint32_t, uint32_t)>(&CreateTask));
	classDef.def("CreateTask", static_cast<void (*)(lua::State *, ai::TaskWrapper &, luabind::object, uint32_t)>(&CreateTask));
	classDef.def("CreateTask", static_cast<void (*)(lua::State *, ai::TaskWrapper &, luabind::object)>(&CreateTask));
	classDef.def("CreateDecoratedTask", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint32_t, luabind::object, uint32_t, uint32_t)>(&CreateDecoratedTask));
	classDef.def("CreateDecoratedTask", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint32_t, luabind::object, uint32_t)>(&CreateDecoratedTask));
	classDef.def("CreateDecoratedTask", static_cast<void (*)(lua::State *, ai::TaskWrapper &, uint32_t, luabind::object)>(&CreateDecoratedTask));

	classDef.add_static_constant("TYPE_SELECTOR", pragma::math::to_integral(pragma::ai::BehaviorNode::Type::Selector));
	classDef.add_static_constant("TYPE_SEQUENCE", pragma::math::to_integral(pragma::ai::BehaviorNode::Type::Sequence));

	classDef.add_static_constant("RESULT_INITIAL", pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Initial));
	classDef.add_static_constant("RESULT_PENDING", pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Pending));
	classDef.add_static_constant("RESULT_FAILED", pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Failed));
	classDef.add_static_constant("RESULT_SUCCEEDED", pragma::math::to_integral(pragma::ai::BehaviorNode::Result::Succeeded));

	classDef.add_static_constant("SELECTOR_TYPE_SEQUENTIAL", pragma::math::to_integral(pragma::ai::SelectorType::Sequential));
	classDef.add_static_constant("SELECTOR_TYPE_RANDOM_SHUFFLE", pragma::math::to_integral(pragma::ai::SelectorType::RandomShuffle));

	classDef.add_static_constant("DECORATOR_TYPE_INHERIT", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::Inherit));
	classDef.add_static_constant("DECORATOR_TYPE_ALWAYS_FAIL", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::AlwaysFail));
	classDef.add_static_constant("DECORATOR_TYPE_ALWAYS_SUCCEED", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::AlwaysSucceed));
	classDef.add_static_constant("DECORATOR_TYPE_INVERT", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::Invert));
	classDef.add_static_constant("DECORATOR_TYPE_LIMIT", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::Limit));
	classDef.add_static_constant("DECORATOR_TYPE_REPEAT", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::Repeat));
	classDef.add_static_constant("DECORATOR_TYPE_UNTIL_FAIL", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::UntilFail));
	classDef.add_static_constant("DECORATOR_TYPE_UNTIL_SUCCESS", pragma::math::to_integral(pragma::ai::TaskDecorator::DecoratorType::UntilSuccess));

	classDef.add_static_constant("PARAMETER_TYPE_NONE", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::None));
	classDef.add_static_constant("PARAMETER_TYPE_BOOL", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::Bool));
	classDef.add_static_constant("PARAMETER_TYPE_INT", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::Int));
	classDef.add_static_constant("PARAMETER_TYPE_FLOAT", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::Float));
	classDef.add_static_constant("PARAMETER_TYPE_STRING", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::String));
	classDef.add_static_constant("PARAMETER_TYPE_VECTOR", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::Vector));
	classDef.add_static_constant("PARAMETER_TYPE_QUATERNION", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::Quaternion));
	classDef.add_static_constant("PARAMETER_TYPE_EULER_ANGLES", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::EulerAngles));
	classDef.add_static_constant("PARAMETER_TYPE_ENTITY", pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::Entity));
	mod[classDef];

	auto classDefBehaviorNode = luabind::class_<ai::BaseBehaviorTask, luabind::bases<ai::TaskWrapper>, luabind::default_holder, AILuaBehaviorNodeWrapper>("BaseBehaviorTask");
	classDefBehaviorNode.def(luabind::constructor<uint32_t, uint32_t>());
	classDefBehaviorNode.def(luabind::constructor<uint32_t>());
	classDefBehaviorNode.def(luabind::constructor<>());
	classDefBehaviorNode.def(luabind::tostring(luabind::self));
	classDefBehaviorNode.def("Start", &AILuaBehaviorNodeWrapper::Start, &AILuaBehaviorNodeWrapper::default_Start);
	classDefBehaviorNode.def("Stop", &AILuaBehaviorNodeWrapper::Stop, &AILuaBehaviorNodeWrapper::default_Stop);
	classDefBehaviorNode.def("Think", &AILuaBehaviorNodeWrapper::Think, &AILuaBehaviorNodeWrapper::default_Think);
	classDefBehaviorNode.def("OnTaskComplete", &AILuaBehaviorNodeWrapper::OnTaskComplete, &AILuaBehaviorNodeWrapper::default_OnTaskComplete);
	classDefBehaviorNode.def("OnSetScheduleParameter", &AILuaBehaviorNodeWrapper::OnSetScheduleParameter, &AILuaBehaviorNodeWrapper::default_OnSetScheduleParameter);
	mod[classDefBehaviorNode];
}

void Lua::AIBehaviorNode::GetType(lua::State *l, ai::TaskWrapper &task) { PushInt(l, pragma::math::to_integral(task->GetType())); }
void Lua::AIBehaviorNode::GetSelectorType(lua::State *l, ai::TaskWrapper &task) { PushInt(l, pragma::math::to_integral(task->GetSelectorType())); }
void Lua::AIBehaviorNode::IsActive(lua::State *l, ai::TaskWrapper &task) { PushBool(l, task->IsActive()); }
void Lua::AIBehaviorNode::GetNode(lua::State *l, ai::TaskWrapper &task, uint32_t nodeId)
{
	auto &nodes = task->GetNodes();
	if(nodeId >= nodes.size())
		return;
	auto &node = nodes.at(nodeId);
	ai::push_task(l, *node);
}
void Lua::AIBehaviorNode::GetNodes(lua::State *l, ai::TaskWrapper &task)
{
	auto &nodes = task->GetNodes();
	auto t = CreateTable(l);
	for(auto i = decltype(nodes.size()) {0}; i < nodes.size(); ++i) {
		auto &node = nodes[i];
		PushInt(l, i + 1);
		ai::push_task(l, *node);
		SetTableValue(l, t);
	}
}
void Lua::AIBehaviorNode::SetScheduleParameter(lua::State *, ai::TaskWrapper &task, uint8_t taskParamId, uint8_t scheduleParamId) { task->SetScheduleParameter(taskParamId, scheduleParamId); }
void Lua::AIBehaviorNode::HasParameter(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { PushBool(l, (task->GetParameter(sched.get(), paramIdx) != nullptr) ? true : false); }
void Lua::AIBehaviorNode::LinkParameter(lua::State *l, ai::TaskWrapper &task, uint8_t paramIdx, ai::TaskWrapper &taskOther) { task->LinkParameter(paramIdx, *taskOther); }
void Lua::AIBehaviorNode::LinkParameter(lua::State *l, ai::TaskWrapper &task, uint8_t paramIdx, ai::TaskWrapper &taskOther, uint8_t paramIdxOther) { task->LinkParameter(paramIdx, *taskOther, paramIdxOther); }

template<typename T>
T get_parameter(Lua::ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, pragma::ai::Schedule::Parameter::Type type, const std::function<T(const pragma::ai::BehaviorParameter *)> &f, const T &def)
{
	auto *param = task->GetParameter(sched.get(), paramIdx);
	if(param == nullptr || param->GetType() != type)
		return def;
	return f(param);
};
void Lua::AIBehaviorNode::GetParameterBool(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, bool def)
{
	auto r = get_parameter<decltype(def)>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::Bool, &pragma::ai::BehaviorParameter::GetBool, def);
	PushBool(l, r);
}
void Lua::AIBehaviorNode::GetParameterBool(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterBool(l, task, sched, paramIdx, false); }
void Lua::AIBehaviorNode::GetParameterInt(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, int32_t def)
{
	auto r = get_parameter<decltype(def)>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::Int, &pragma::ai::BehaviorParameter::GetInt, def);
	PushInt(l, r);
}
void Lua::AIBehaviorNode::GetParameterInt(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterInt(l, task, sched, paramIdx, 0); }
void Lua::AIBehaviorNode::GetParameterFloat(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, float def)
{
	auto r = get_parameter<decltype(def)>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::Float, &pragma::ai::BehaviorParameter::GetFloat, def);
	PushNumber(l, r);
}
void Lua::AIBehaviorNode::GetParameterFloat(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterFloat(l, task, sched, paramIdx, 0.f); }
void Lua::AIBehaviorNode::GetParameterString(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const std::string &def)
{
	auto *r = get_parameter<decltype(&def)>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::String, &pragma::ai::BehaviorParameter::GetString, &def);
	PushString(l, *r);
}
void Lua::AIBehaviorNode::GetParameterString(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterString(l, task, sched, paramIdx, ""); }
void Lua::AIBehaviorNode::GetParameterEntity(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, pragma::ecs::BaseEntity *def)
{
	//LUA_CHECK_ENTITY(l,def);
	auto *r = get_parameter<const pragma::ecs::BaseEntity *>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::Entity, &pragma::ai::BehaviorParameter::GetEntity, def);
	if(r != nullptr)
		const_cast<pragma::ecs::BaseEntity *>(r)->GetLuaObject().push(l);
}
void Lua::AIBehaviorNode::GetParameterEntity(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterEntity(l, task, sched, paramIdx, nullptr); }
void Lua::AIBehaviorNode::GetParameterVector(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const Vector3 &def)
{
	auto *r = get_parameter<decltype(&def)>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::Vector, &pragma::ai::BehaviorParameter::GetVector, &def);
	Lua::Push<Vector3>(l, *r);
}
void Lua::AIBehaviorNode::GetParameterVector(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterVector(l, task, sched, paramIdx, {}); }
void Lua::AIBehaviorNode::GetParameterQuaternion(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const Quat &def)
{
	auto *r = get_parameter<decltype(&def)>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::Quaternion, &pragma::ai::BehaviorParameter::GetQuaternion, &def);
	Lua::Push<Quat>(l, *r);
}
void Lua::AIBehaviorNode::GetParameterQuaternion(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterQuaternion(l, task, sched, paramIdx, {}); }
void Lua::AIBehaviorNode::GetParameterEulerAngles(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx, const EulerAngles &def)
{
	auto *r = get_parameter<decltype(&def)>(task, sched, paramIdx, pragma::ai::Schedule::Parameter::Type::EulerAngles, &pragma::ai::BehaviorParameter::GetEulerAngles, &def);
	Lua::Push<EulerAngles>(l, *r);
}
void Lua::AIBehaviorNode::GetParameterEulerAngles(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx) { GetParameterEulerAngles(l, task, sched, paramIdx, {}); }
void Lua::AIBehaviorNode::GetParameterType(lua::State *l, ai::TaskWrapper &task, std::shared_ptr<pragma::ai::Schedule> &sched, uint8_t paramIdx)
{
	auto *param = task->GetParameter(sched.get(), paramIdx);
	if(param == nullptr)
		PushInt(l, pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::None));
	else
		PushInt(l, pragma::math::to_integral(param->GetType()));
}
void Lua::AIBehaviorNode::SetDebugName(lua::State *l, ai::TaskWrapper &task, const std::string &name) { task->SetDebugName(name); }
void Lua::AIBehaviorNode::GetDebugName(lua::State *l, ai::TaskWrapper &task) { PushString(l, task->GetDebugInfo().debugName); }
void Lua::AIBehaviorNode::SetParameterBool(lua::State *, ai::TaskWrapper &task, uint8_t idx, bool b) { task->SetParameter(idx, b); }
void Lua::AIBehaviorNode::SetParameterInt(lua::State *, ai::TaskWrapper &task, uint8_t idx, int32_t i) { task->SetParameter(idx, i); }
void Lua::AIBehaviorNode::SetParameterFloat(lua::State *, ai::TaskWrapper &task, uint8_t idx, float f) { task->SetParameter(idx, f); }
void Lua::AIBehaviorNode::SetParameterString(lua::State *, ai::TaskWrapper &task, uint8_t idx, const std::string &s) { task->SetParameter(idx, s); }
void Lua::AIBehaviorNode::SetParameterEntity(lua::State *l, ai::TaskWrapper &task, uint8_t idx, pragma::ecs::BaseEntity &ent) { task->SetParameter(idx, &ent); }
void Lua::AIBehaviorNode::SetParameterVector(lua::State *, ai::TaskWrapper &task, uint8_t idx, const Vector3 &v) { task->SetParameter(idx, v); }
void Lua::AIBehaviorNode::SetParameterQuaternion(lua::State *, ai::TaskWrapper &task, uint8_t idx, const Quat &rot) { task->SetParameter(idx, rot); }
void Lua::AIBehaviorNode::SetParameterEulerAngles(lua::State *, ai::TaskWrapper &task, uint8_t idx, const EulerAngles &ang) { task->SetParameter(idx, ang); }

static void create_task(lua::State *l, pragma::ai::BehaviorNode &task, luabind::object o, pragma::ai::BehaviorNode::Type *taskType, pragma::ai::SelectorType *selectorType, int32_t aiTaskTypeParamOffset)
{
	if(Lua::IsUserData(l, aiTaskTypeParamOffset) == true) {
		auto oClass = luabind::object(luabind::from_stack(l, aiTaskTypeParamOffset)); // Class Definition

		auto newTask = Lua::ai::server::create_lua_task(l, oClass, (taskType != nullptr) ? *taskType : pragma::ai::BehaviorNode::Type::Sequence, (selectorType != nullptr) ? *selectorType : pragma::ai::SelectorType::Sequential);
		if(newTask == nullptr)
			return;
		Lua::ai::push_task(l, *newTask);
		task.AddNode(newTask);
	}
	else {
		auto taskId = Lua::CheckInt(l, aiTaskTypeParamOffset);
		auto &taskManager = pragma::SGame::Get()->GetAITaskManager();
		auto newTask = taskManager.CreateTask(static_cast<uint32_t>(taskId));

		if(taskType != nullptr)
			newTask->SetType(*taskType);
		if(selectorType != nullptr)
			newTask->SetSelectorType(*selectorType);

		Lua::ai::push_task(l, *newTask);
		task.AddNode(newTask);
	}
}
void Lua::AIBehaviorNode::CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o, pragma::ai::BehaviorNode::Type *taskType, pragma::ai::SelectorType *selectorType, int32_t aiTaskTypeParamOffset) { create_task(l, *task, o, taskType, selectorType, aiTaskTypeParamOffset); }
void Lua::AIBehaviorNode::CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o, uint32_t taskType, uint32_t selectorType)
{
	auto ttaskType = static_cast<pragma::ai::BehaviorNode::Type>(taskType);
	auto tselectorType = static_cast<pragma::ai::SelectorType>(selectorType);
	CreateTask(l, task, o, &ttaskType, &tselectorType);
}
void Lua::AIBehaviorNode::CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o, uint32_t taskType)
{
	auto ttaskType = static_cast<pragma::ai::BehaviorNode::Type>(taskType);
	CreateTask(l, task, o, &ttaskType, nullptr);
}
void Lua::AIBehaviorNode::CreateTask(lua::State *l, ai::TaskWrapper &task, luabind::object o) { CreateTask(l, task, o, nullptr, nullptr); }

void Lua::AIBehaviorNode::CreateDecoratedTask(lua::State *l, ai::TaskWrapper &task, uint32_t decoratorType, luabind::object o, uint32_t taskType, uint32_t selectorType)
{
	auto &taskManager = pragma::SGame::Get()->GetAITaskManager();
	auto newTask = taskManager.CreateTask(pragma::math::to_integral(pragma::ai::Task::Decorator));
	task->AddNode(newTask);
	newTask->SetParameter(0, static_cast<int32_t>(decoratorType));

	auto ttaskType = static_cast<pragma::ai::BehaviorNode::Type>(taskType);
	auto tselectorType = static_cast<pragma::ai::SelectorType>(selectorType);
	create_task(l, *newTask, o, &ttaskType, &tselectorType, 3);
}
void Lua::AIBehaviorNode::CreateDecoratedTask(lua::State *l, ai::TaskWrapper &task, uint32_t decoratorType, luabind::object o, uint32_t taskType)
{
	auto &taskManager = pragma::SGame::Get()->GetAITaskManager();
	auto newTask = taskManager.CreateTask(pragma::math::to_integral(pragma::ai::Task::Decorator));
	task->AddNode(newTask);
	newTask->SetParameter(0, static_cast<int32_t>(decoratorType));

	auto ttaskType = static_cast<pragma::ai::BehaviorNode::Type>(taskType);
	create_task(l, *newTask, o, &ttaskType, nullptr, 3);
}
void Lua::AIBehaviorNode::CreateDecoratedTask(lua::State *l, ai::TaskWrapper &task, uint32_t decoratorType, luabind::object o)
{
	auto &taskManager = pragma::SGame::Get()->GetAITaskManager();
	auto newTask = taskManager.CreateTask(pragma::math::to_integral(pragma::ai::Task::Decorator));
	task->AddNode(newTask);
	newTask->SetParameter(0, static_cast<int32_t>(decoratorType));

	create_task(l, *newTask, o, nullptr, nullptr, 3);
}
