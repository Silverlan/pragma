/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/s_lai_schedule.h"
#include "pragma/lua/classes/s_lai_behavior.h"
#include "pragma/lua/classes/s_lentity.h"
#include "pragma/ai/ai_schedule.h"
#include "luasystem.h"

namespace Lua
{
	namespace AISchedule
	{
		static void AddTask(lua_State *l,pragma::ai::Schedule &schedule,::pragma::ai::BehaviorNode &node);
		static void GetRootNode(lua_State *l,pragma::ai::Schedule &schedule);
		static void Cancel(lua_State *l,pragma::ai::Schedule &schedule);
		static void Copy(lua_State *l,pragma::ai::Schedule &schedule);
		static void HasParameter(lua_State *l,pragma::ai::Schedule &schedule,uint32_t parameterId);
		static void SetParameterBool(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,bool b);
		static void SetParameterInt(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,int32_t i);
		static void SetParameterFloat(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,float f);
		static void SetParameterString(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,const std::string &s);
		static void SetParameterEntity(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,EntityHandle &hEnt);
		static void SetParameterVector(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,const Vector3 &v);
		static void SetParameterQuaternion(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,const Quat &rot);
		static void SetParameterEulerAngles(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,const EulerAngles &ang);

		static void GetParameterBool(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void GetParameterInt(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void GetParameterFloat(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void GetParameterString(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void GetParameterEntity(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void GetParameterVector(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void GetParameterQuaternion(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void GetParameterEulerAngles(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);

		static void GetParameterBool(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,bool def);
		static void GetParameterInt(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,int32_t def);
		static void GetParameterFloat(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,float def);
		static void GetParameterString(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const std::string &def);
		static void GetParameterEntity(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,EntityHandle &def);
		static void GetParameterVector(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const Vector3 &def);
		static void GetParameterQuaternion(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const Quat &def);
		static void GetParameterEulerAngles(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const EulerAngles &def);

		static void GetParameterType(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx);
		static void DebugPrint(lua_State *l,pragma::ai::Schedule &schedule);

		static void SetInterruptFlags(lua_State *l,pragma::ai::Schedule &schedule,uint8_t flags);
		static void AddInterruptFlags(lua_State *l,pragma::ai::Schedule &schedule,uint8_t flags);
		static void GetInterruptFlags(lua_State *l,pragma::ai::Schedule &schedule);
	};
};

void Lua::AISchedule::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::ai::Schedule>("Schedule");
	classDef.def("AddTask",&AddTask);
	classDef.def("GetRootTask",&GetRootNode);
	classDef.def("Cancel",&Cancel);
	classDef.def("Copy",&Copy);
	classDef.def("HasParameter",&HasParameter);
	classDef.def("SetParameterBool",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,bool)>(&SetParameterBool));
	classDef.def("SetParameterInt",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,int32_t)>(&SetParameterInt));
	classDef.def("SetParameterFloat",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,float)>(&SetParameterFloat));
	classDef.def("SetParameterString",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const std::string&)>(&SetParameterString));
	classDef.def("SetParameterEntity",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,EntityHandle&)>(&SetParameterEntity));
	classDef.def("SetParameterVector",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const Vector3&)>(&SetParameterVector));
	classDef.def("SetParameterQuaternion",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const Quat&)>(&SetParameterQuaternion));
	classDef.def("SetParameterEulerAngles",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const EulerAngles&)>(&SetParameterEulerAngles));

	classDef.def("GetParameterBool",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,bool)>(&GetParameterBool));
	classDef.def("GetParameterBool",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterBool));
	classDef.def("GetParameterInt",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,int32_t)>(&GetParameterInt));
	classDef.def("GetParameterInt",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterInt));
	classDef.def("GetParameterFloat",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,float)>(&GetParameterFloat));
	classDef.def("GetParameterFloat",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterFloat));
	classDef.def("GetParameterString",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const std::string&)>(&GetParameterString));
	classDef.def("GetParameterString",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterString));
	classDef.def("GetParameterEntity",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,EntityHandle&)>(&GetParameterEntity));
	classDef.def("GetParameterEntity",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterEntity));
	classDef.def("GetParameterVector",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const Vector3&)>(&GetParameterVector));
	classDef.def("GetParameterVector",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterVector));
	classDef.def("GetParameterQuaternion",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const Quat&)>(&GetParameterQuaternion));
	classDef.def("GetParameterQuaternion",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterQuaternion));
	classDef.def("GetParameterEulerAngles",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t,const EulerAngles&)>(&GetParameterEulerAngles));
	classDef.def("GetParameterEulerAngles",static_cast<void(*)(lua_State*,pragma::ai::Schedule&,uint8_t)>(&GetParameterEulerAngles));

	classDef.def("GetParameterType",&GetParameterType);
	classDef.def("DebugPrint",&DebugPrint);

	classDef.def("SetInterruptFlags",&SetInterruptFlags);
	classDef.def("AddInterruptFlags",&AddInterruptFlags);
	classDef.def("GetInterruptFlags",&GetInterruptFlags);
	mod[classDef];
}

void Lua::AISchedule::AddTask(lua_State*,pragma::ai::Schedule &schedule,::pragma::ai::BehaviorNode &node)
{
	schedule.GetRootNode().AddNode(node.shared_from_this());
}
void Lua::AISchedule::GetRootNode(lua_State *l,pragma::ai::Schedule &schedule)
{
	auto &rootNode = schedule.GetRootNode();
	Lua::ai::push_task(l,rootNode);
}
void Lua::AISchedule::Cancel(lua_State*,pragma::ai::Schedule &schedule) {schedule.Cancel();}
void Lua::AISchedule::Copy(lua_State *l,pragma::ai::Schedule &schedule)
{
	auto cpy = schedule.Copy();
	Lua::Push<std::shared_ptr<pragma::ai::Schedule>>(l,cpy);
}
void Lua::AISchedule::HasParameter(lua_State *l,pragma::ai::Schedule &schedule,uint32_t parameterId)
{
	Lua::PushBool(l,(schedule.GetParameter(parameterId) != nullptr) ? true : false);
}
void Lua::AISchedule::SetParameterBool(lua_State*,pragma::ai::Schedule &schedule,uint8_t idx,bool b) {schedule.SetParameter(idx,b);}
void Lua::AISchedule::SetParameterInt(lua_State*,pragma::ai::Schedule &schedule,uint8_t idx,int32_t i) {schedule.SetParameter(idx,i);}
void Lua::AISchedule::SetParameterFloat(lua_State*,pragma::ai::Schedule &schedule,uint8_t idx,float f) {schedule.SetParameter(idx,f);}
void Lua::AISchedule::SetParameterString(lua_State*,pragma::ai::Schedule &schedule,uint8_t idx,const std::string &s) {schedule.SetParameter(idx,s);}
void Lua::AISchedule::SetParameterEntity(lua_State *l,pragma::ai::Schedule &schedule,uint8_t idx,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	schedule.SetParameter(idx,hEnt.get());
}
void Lua::AISchedule::SetParameterVector(lua_State*,pragma::ai::Schedule &schedule,uint8_t idx,const Vector3 &v) {schedule.SetParameter(idx,v);}
void Lua::AISchedule::SetParameterQuaternion(lua_State*,pragma::ai::Schedule &schedule,uint8_t idx,const Quat &rot) {schedule.SetParameter(idx,rot);}
void Lua::AISchedule::SetParameterEulerAngles(lua_State*,pragma::ai::Schedule &schedule,uint8_t idx,const EulerAngles &ang) {schedule.SetParameter(idx,ang);}

template<typename T>
	T get_parameter(pragma::ai::Schedule &schedule,uint8_t paramIdx,::pragma::ai::Schedule::Parameter::Type type,const std::function<T(const ::pragma::ai::BehaviorParameter*)> &f,const T &def)
{
	auto *param = schedule.GetParameter(paramIdx);
	if(param == nullptr || param->GetType() != type)
		return def;
	return f(param);
};
void Lua::AISchedule::GetParameterBool(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,bool def)
{
	auto r = get_parameter<decltype(def)>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::Bool,&::pragma::ai::BehaviorParameter::GetBool,def);
	Lua::PushBool(l,r);
}
void Lua::AISchedule::GetParameterBool(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx) {GetParameterBool(l,schedule,paramIdx,false);}
void Lua::AISchedule::GetParameterInt(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,int32_t def)
{
	auto r = get_parameter<decltype(def)>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::Int,&::pragma::ai::BehaviorParameter::GetInt,def);
	Lua::PushInt(l,r);
}
void Lua::AISchedule::GetParameterInt(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx) {GetParameterInt(l,schedule,paramIdx,0);}
void Lua::AISchedule::GetParameterFloat(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,float def)
{
	auto r = get_parameter<decltype(def)>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::Float,&::pragma::ai::BehaviorParameter::GetFloat,def);
	Lua::PushNumber(l,r);
}
void Lua::AISchedule::GetParameterFloat(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx) {GetParameterFloat(l,schedule,paramIdx,0.f);}
void Lua::AISchedule::GetParameterString(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const std::string &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::String,&::pragma::ai::BehaviorParameter::GetString,&def);
	Lua::PushString(l,*r);
}
void Lua::AISchedule::GetParameterString(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx) {GetParameterString(l,schedule,paramIdx,"");}
void Lua::AISchedule::GetParameterEntity(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,EntityHandle &def)
{
	//LUA_CHECK_ENTITY(l,def);
	auto *r = get_parameter<const BaseEntity*>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::Entity,&::pragma::ai::BehaviorParameter::GetEntity,def.get());
	if(r != nullptr)
		const_cast<BaseEntity*>(r)->GetLuaObject()->push(l);
}
void Lua::AISchedule::GetParameterEntity(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx)
{
	EntityHandle def {};
	GetParameterEntity(l,schedule,paramIdx,def);
}
void Lua::AISchedule::GetParameterVector(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const Vector3 &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::Vector,&::pragma::ai::BehaviorParameter::GetVector,&def);
	Lua::Push<Vector3>(l,*r);
}
void Lua::AISchedule::GetParameterVector(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx) {GetParameterVector(l,schedule,paramIdx,{});}
void Lua::AISchedule::GetParameterQuaternion(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const Quat &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::Quaternion,&::pragma::ai::BehaviorParameter::GetQuaternion,&def);
	Lua::Push<Quat>(l,*r);
}
void Lua::AISchedule::GetParameterQuaternion(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx) {GetParameterQuaternion(l,schedule,paramIdx,{});}
void Lua::AISchedule::GetParameterEulerAngles(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx,const EulerAngles &def)
{
	auto *r = get_parameter<decltype(&def)>(schedule,paramIdx,::pragma::ai::Schedule::Parameter::Type::EulerAngles,&::pragma::ai::BehaviorParameter::GetEulerAngles,&def);
	Lua::Push<EulerAngles>(l,*r);
}
void Lua::AISchedule::GetParameterEulerAngles(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx) {GetParameterEulerAngles(l,schedule,paramIdx,{});}
void Lua::AISchedule::GetParameterType(lua_State *l,pragma::ai::Schedule &schedule,uint8_t paramIdx)
{
	auto *p = schedule.GetParameter(paramIdx);
	if(p == nullptr)
		Lua::PushInt(l,umath::to_integral(pragma::ai::Schedule::Parameter::Type::None));
	else
		Lua::PushInt(l,umath::to_integral(p->GetType()));
}
void Lua::AISchedule::DebugPrint(lua_State *l,pragma::ai::Schedule &schedule)
{
	std::stringstream ss;
	schedule.DebugPrint(ss);
	Con::cout<<ss.str()<<Con::endl;
}
void Lua::AISchedule::SetInterruptFlags(lua_State *l,pragma::ai::Schedule &schedule,uint8_t flags)
{
	schedule.SetInterruptFlags(flags);
}
void Lua::AISchedule::AddInterruptFlags(lua_State *l,pragma::ai::Schedule &schedule,uint8_t flags)
{
	schedule.AddInterruptFlags(flags);
}
void Lua::AISchedule::GetInterruptFlags(lua_State *l,pragma::ai::Schedule &schedule)
{
	Lua::PushInt(l,schedule.GetInterruptFlags());
}
