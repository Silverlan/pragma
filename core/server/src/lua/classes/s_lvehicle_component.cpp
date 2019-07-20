#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/entities/components/s_vehicle_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lua_entity_component.hpp>

namespace Lua
{
	namespace Vehicle
	{
		namespace Server
		{
			static void AddWheel(lua_State *l,SVehicleHandle &hEnt,const std::string &mdl,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset,const Quat &mdlRotation);
			static void AddWheel(lua_State *l,SVehicleHandle &hEnt,const std::string &mdl,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel);
			static void AddWheel(lua_State *l,SVehicleHandle &hEnt,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset,const Quat &mdlRotation);
			static void AddWheel(lua_State *l,SVehicleHandle &hEnt,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel);
			static void SetSteeringWheelModel(lua_State *l,SVehicleHandle &hEnt,const std::string &mdl);
			static void IsFirstPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt);
			static void SetFirstPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt,bool b);
			static void IsThirdPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt);
			static void SetThirdPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt,bool b);
		};
	};
};
void Lua::register_sv_vehicle_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<SVehicleHandle,BaseEntityComponentHandle>("VehicleComponent");
	Lua::register_base_vehicle_component_methods<luabind::class_<SVehicleHandle,BaseEntityComponentHandle>,SVehicleHandle>(l,def);
	def.def("AddWheel",static_cast<void(*)(lua_State*,SVehicleHandle&,const std::string&,Vector3&,Vector3&,Bool,const Vector3&,const Quat&)>(&Lua::Vehicle::Server::AddWheel));
	def.def("AddWheel",static_cast<void(*)(lua_State*,SVehicleHandle&,const std::string&,Vector3&,Vector3&,Bool)>(&Lua::Vehicle::Server::AddWheel));
	def.def("AddWheel",static_cast<void(*)(lua_State*,SVehicleHandle&,Vector3&,Vector3&,Bool,const Vector3&,const Quat&)>(&Lua::Vehicle::Server::AddWheel));
	def.def("AddWheel",static_cast<void(*)(lua_State*,SVehicleHandle&,Vector3&,Vector3&,Bool)>(&Lua::Vehicle::Server::AddWheel));
	def.def("SetSteeringWheelModel",&Lua::Vehicle::Server::SetSteeringWheelModel);
	def.def("IsFirstPersonCameraEnabled",&Lua::Vehicle::Server::IsFirstPersonCameraEnabled);
	def.def("SetFirstPersonCameraEnabled",&Lua::Vehicle::Server::SetFirstPersonCameraEnabled);
	def.def("IsThirdPersonCameraEnabled",&Lua::Vehicle::Server::IsThirdPersonCameraEnabled);
	def.def("SetThirdPersonCameraEnabled",&Lua::Vehicle::Server::SetThirdPersonCameraEnabled);
	module[def];
}
void Lua::Vehicle::Server::AddWheel(lua_State *l,SVehicleHandle &hEnt,const std::string &mdl,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset,const Quat &mdlRotation)
{
	pragma::Lua::check_component(l,hEnt);
	auto *ent = hEnt->AddWheel(mdl,connectionPoint,wheelAxle,bIsFrontWheel,mdlOffset,mdlRotation);
	if(ent == nullptr)
		return;
	lua_pushentity(l,ent);
}
void Lua::Vehicle::Server::AddWheel(lua_State *l,SVehicleHandle &hEnt,const std::string &mdl,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel) {Lua::Vehicle::Server::AddWheel(l,hEnt,mdl,connectionPoint,wheelAxle,bIsFrontWheel,{},{});}
void Lua::Vehicle::Server::AddWheel(lua_State *l,SVehicleHandle &hEnt,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset,const Quat &mdlRotation)
{
	Lua::Vehicle::Server::AddWheel(l,hEnt,"",connectionPoint,wheelAxle,bIsFrontWheel,mdlOffset,mdlRotation);
}
void Lua::Vehicle::Server::AddWheel(lua_State *l,SVehicleHandle &hEnt,Vector3 &connectionPoint,Vector3 &wheelAxle,Bool bIsFrontWheel) {Lua::Vehicle::Server::AddWheel(l,hEnt,connectionPoint,wheelAxle,bIsFrontWheel,{},{});}

void Lua::Vehicle::Server::SetSteeringWheelModel(lua_State *l,SVehicleHandle &hEnt,const std::string &mdl)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetSteeringWheelModel(mdl);
}
void Lua::Vehicle::Server::IsFirstPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsFirstPersonCameraEnabled());
}
void Lua::Vehicle::Server::SetFirstPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt,bool b)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetFirstPersonCameraEnabled(b);
}
void Lua::Vehicle::Server::IsThirdPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsThirdPersonCameraEnabled());
}
void Lua::Vehicle::Server::SetThirdPersonCameraEnabled(lua_State *l,SVehicleHandle &hEnt,bool b)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetThirdPersonCameraEnabled(b);
}
