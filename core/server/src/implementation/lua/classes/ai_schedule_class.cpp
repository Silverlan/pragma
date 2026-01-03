// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.ai_schedule;

import :ai.schedule;
import :scripting.lua.classes.ai_behavior;
import :scripting.lua.classes.entity;

namespace Lua {
	namespace AISchedule {
		static void AddTask(lua::State *l, pragma::ai::Schedule &schedule, pragma::ai::BehaviorNode &node);
		static void GetRootNode(lua::State *l, pragma::ai::Schedule &schedule);
		static void Cancel(lua::State *l, pragma::ai::Schedule &schedule);
		static void Copy(lua::State *l, pragma::ai::Schedule &schedule);
		static void HasParameter(lua::State *l, pragma::ai::Schedule &schedule, uint32_t parameterId);
		static void SetParameterBool(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, bool b);
		static void SetParameterInt(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, int32_t i);
		static void SetParameterFloat(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, float f);
		static void SetParameterString(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, const std::string &s);
		static void SetParameterEntity(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, pragma::ecs::BaseEntity &hEnt);
		static void SetParameterVector(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, const Vector3 &v);
		static void SetParameterQuaternion(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, const Quat &rot);
		static void SetParameterEulerAngles(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, const EulerAngles &ang);

		static void GetParameterBool(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void GetParameterInt(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void GetParameterFloat(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void GetParameterString(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void GetParameterEntity(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void GetParameterVector(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void GetParameterQuaternion(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void GetParameterEulerAngles(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);

		static void GetParameterBool(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, bool def);
		static void GetParameterInt(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, int32_t def);
		static void GetParameterFloat(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, float def);
		static void GetParameterString(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const std::string &def);
		static void GetParameterEntity(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, EntityHandle &def);
		static void GetParameterVector(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const Vector3 &def);
		static void GetParameterQuaternion(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const Quat &def);
		static void GetParameterEulerAngles(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const EulerAngles &def);

		static void GetParameterType(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx);
		static void DebugPrint(lua::State *l, pragma::ai::Schedule &schedule);

		static void SetInterruptFlags(lua::State *l, pragma::ai::Schedule &schedule, uint8_t flags);
		static void AddInterruptFlags(lua::State *l, pragma::ai::Schedule &schedule, uint8_t flags);
		static void GetInterruptFlags(lua::State *l, pragma::ai::Schedule &schedule);
	};
};

void Lua::AISchedule::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::ai::Schedule>("Schedule");
	classDef.def("AddTask", &AddTask);
	classDef.def("GetRootTask", &GetRootNode);
	classDef.def("Cancel", &Cancel);
	classDef.def("Copy", &Copy);
	classDef.def("HasParameter", &HasParameter);
	classDef.def("SetParameterBool", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, bool)>(&SetParameterBool));
	classDef.def("SetParameterInt", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, int32_t)>(&SetParameterInt));
	classDef.def("SetParameterFloat", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, float)>(&SetParameterFloat));
	classDef.def("SetParameterString", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const std::string &)>(&SetParameterString));
	classDef.def("SetParameterEntity", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, pragma::ecs::BaseEntity &)>(&SetParameterEntity));
	classDef.def("SetParameterVector", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const Vector3 &)>(&SetParameterVector));
	classDef.def("SetParameterQuaternion", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const Quat &)>(&SetParameterQuaternion));
	classDef.def("SetParameterEulerAngles", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const EulerAngles &)>(&SetParameterEulerAngles));

	classDef.def("GetParameterBool", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, bool)>(&GetParameterBool));
	classDef.def("GetParameterBool", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterBool));
	classDef.def("GetParameterInt", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, int32_t)>(&GetParameterInt));
	classDef.def("GetParameterInt", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterInt));
	classDef.def("GetParameterFloat", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, float)>(&GetParameterFloat));
	classDef.def("GetParameterFloat", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterFloat));
	classDef.def("GetParameterString", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const std::string &)>(&GetParameterString));
	classDef.def("GetParameterString", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterString));
	classDef.def("GetParameterEntity", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, EntityHandle &)>(&GetParameterEntity));
	classDef.def("GetParameterEntity", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterEntity));
	classDef.def("GetParameterVector", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const Vector3 &)>(&GetParameterVector));
	classDef.def("GetParameterVector", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterVector));
	classDef.def("GetParameterQuaternion", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const Quat &)>(&GetParameterQuaternion));
	classDef.def("GetParameterQuaternion", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterQuaternion));
	classDef.def("GetParameterEulerAngles", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t, const EulerAngles &)>(&GetParameterEulerAngles));
	classDef.def("GetParameterEulerAngles", static_cast<void (*)(lua::State *, pragma::ai::Schedule &, uint8_t)>(&GetParameterEulerAngles));

	classDef.def("GetParameterType", &GetParameterType);
	classDef.def("DebugPrint", &DebugPrint);

	classDef.def("SetInterruptFlags", &SetInterruptFlags);
	classDef.def("AddInterruptFlags", &AddInterruptFlags);
	classDef.def("GetInterruptFlags", &GetInterruptFlags);
	mod[classDef];
}

void Lua::AISchedule::AddTask(lua::State *, pragma::ai::Schedule &schedule, pragma::ai::BehaviorNode &node) { schedule.GetRootNode().AddNode(node.shared_from_this()); }
void Lua::AISchedule::GetRootNode(lua::State *l, pragma::ai::Schedule &schedule)
{
	auto &rootNode = schedule.GetRootNode();
	ai::push_task(l, rootNode);
}
void Lua::AISchedule::Cancel(lua::State *, pragma::ai::Schedule &schedule) { schedule.Cancel(); }
void Lua::AISchedule::Copy(lua::State *l, pragma::ai::Schedule &schedule)
{
	auto cpy = schedule.Copy();
	Lua::Push<std::shared_ptr<pragma::ai::Schedule>>(l, cpy);
}
void Lua::AISchedule::HasParameter(lua::State *l, pragma::ai::Schedule &schedule, uint32_t parameterId) { PushBool(l, (schedule.GetParameter(parameterId) != nullptr) ? true : false); }
void Lua::AISchedule::SetParameterBool(lua::State *, pragma::ai::Schedule &schedule, uint8_t idx, bool b) { schedule.SetParameter(idx, b); }
void Lua::AISchedule::SetParameterInt(lua::State *, pragma::ai::Schedule &schedule, uint8_t idx, int32_t i) { schedule.SetParameter(idx, i); }
void Lua::AISchedule::SetParameterFloat(lua::State *, pragma::ai::Schedule &schedule, uint8_t idx, float f) { schedule.SetParameter(idx, f); }
void Lua::AISchedule::SetParameterString(lua::State *, pragma::ai::Schedule &schedule, uint8_t idx, const std::string &s) { schedule.SetParameter(idx, s); }
void Lua::AISchedule::SetParameterEntity(lua::State *l, pragma::ai::Schedule &schedule, uint8_t idx, pragma::ecs::BaseEntity &ent) { schedule.SetParameter(idx, &ent); }
void Lua::AISchedule::SetParameterVector(lua::State *, pragma::ai::Schedule &schedule, uint8_t idx, const Vector3 &v) { schedule.SetParameter(idx, v); }
void Lua::AISchedule::SetParameterQuaternion(lua::State *, pragma::ai::Schedule &schedule, uint8_t idx, const Quat &rot) { schedule.SetParameter(idx, rot); }
void Lua::AISchedule::SetParameterEulerAngles(lua::State *, pragma::ai::Schedule &schedule, uint8_t idx, const EulerAngles &ang) { schedule.SetParameter(idx, ang); }

template<typename T>
T get_parameter(pragma::ai::Schedule &schedule, uint8_t paramIdx, pragma::ai::Schedule::Parameter::Type type, const std::function<T(const pragma::ai::BehaviorParameter *)> &f, const T &def)
{
	auto *param = schedule.GetParameter(paramIdx);
	if(param == nullptr || param->GetType() != type)
		return def;
	return f(param);
};
void Lua::AISchedule::GetParameterBool(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, bool def)
{
	auto r = get_parameter<decltype(def)>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::Bool, &pragma::ai::BehaviorParameter::GetBool, def);
	PushBool(l, r);
}
void Lua::AISchedule::GetParameterBool(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx) { GetParameterBool(l, schedule, paramIdx, false); }
void Lua::AISchedule::GetParameterInt(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, int32_t def)
{
	auto r = get_parameter<decltype(def)>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::Int, &pragma::ai::BehaviorParameter::GetInt, def);
	PushInt(l, r);
}
void Lua::AISchedule::GetParameterInt(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx) { GetParameterInt(l, schedule, paramIdx, 0); }
void Lua::AISchedule::GetParameterFloat(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, float def)
{
	auto r = get_parameter<decltype(def)>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::Float, &pragma::ai::BehaviorParameter::GetFloat, def);
	PushNumber(l, r);
}
void Lua::AISchedule::GetParameterFloat(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx) { GetParameterFloat(l, schedule, paramIdx, 0.f); }
void Lua::AISchedule::GetParameterString(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const std::string &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::String, &pragma::ai::BehaviorParameter::GetString, &def);
	PushString(l, *r);
}
void Lua::AISchedule::GetParameterString(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx) { GetParameterString(l, schedule, paramIdx, ""); }
void Lua::AISchedule::GetParameterEntity(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, EntityHandle &def)
{
	//LUA_CHECK_ENTITY(l,def);
	auto *r = get_parameter<const pragma::ecs::BaseEntity *>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::Entity, &pragma::ai::BehaviorParameter::GetEntity, def.get());
	if(r != nullptr)
		const_cast<pragma::ecs::BaseEntity *>(r)->GetLuaObject().push(l);
}
void Lua::AISchedule::GetParameterEntity(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx)
{
	EntityHandle def {};
	GetParameterEntity(l, schedule, paramIdx, def);
}
void Lua::AISchedule::GetParameterVector(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const Vector3 &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::Vector, &pragma::ai::BehaviorParameter::GetVector, &def);
	Lua::Push<Vector3>(l, *r);
}
void Lua::AISchedule::GetParameterVector(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx) { GetParameterVector(l, schedule, paramIdx, {}); }
void Lua::AISchedule::GetParameterQuaternion(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const Quat &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::Quaternion, &pragma::ai::BehaviorParameter::GetQuaternion, &def);
	Lua::Push<Quat>(l, *r);
}
void Lua::AISchedule::GetParameterQuaternion(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx) { GetParameterQuaternion(l, schedule, paramIdx, {}); }
void Lua::AISchedule::GetParameterEulerAngles(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx, const EulerAngles &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule, paramIdx, pragma::ai::Schedule::Parameter::Type::EulerAngles, &pragma::ai::BehaviorParameter::GetEulerAngles, &def);
	Lua::Push<EulerAngles>(l, *r);
}
void Lua::AISchedule::GetParameterEulerAngles(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx) { GetParameterEulerAngles(l, schedule, paramIdx, {}); }
void Lua::AISchedule::GetParameterType(lua::State *l, pragma::ai::Schedule &schedule, uint8_t paramIdx)
{
	auto *p = schedule.GetParameter(paramIdx);
	if(p == nullptr)
		PushInt(l, pragma::math::to_integral(pragma::ai::Schedule::Parameter::Type::None));
	else
		PushInt(l, pragma::math::to_integral(p->GetType()));
}
void Lua::AISchedule::DebugPrint(lua::State *l, pragma::ai::Schedule &schedule)
{
	std::stringstream ss;
	schedule.DebugPrint(ss);
	Con::COUT << ss.str() << Con::endl;
}
void Lua::AISchedule::SetInterruptFlags(lua::State *l, pragma::ai::Schedule &schedule, uint8_t flags) { schedule.SetInterruptFlags(flags); }
void Lua::AISchedule::AddInterruptFlags(lua::State *l, pragma::ai::Schedule &schedule, uint8_t flags) { schedule.AddInterruptFlags(flags); }
void Lua::AISchedule::GetInterruptFlags(lua::State *l, pragma::ai::Schedule &schedule) { PushInt(l, schedule.GetInterruptFlags()); }
