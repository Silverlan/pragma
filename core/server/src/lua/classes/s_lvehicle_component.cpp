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
			static void SetWheelDirection(lua_State *l,SVehicleHandle &hEnt,const Vector3 &dir);
			static void SetMaxSuspensionLength(lua_State *l,SVehicleHandle &hEnt,Float len);
			static void SetMaxSuspensionCompression(lua_State *l,SVehicleHandle &hEnt,Float cmp);
			static void SetWheelRadius(lua_State *l,SVehicleHandle &hEnt,Float radius);
			static void SetSuspensionStiffness(lua_State *l,SVehicleHandle &hEnt,Float stiffness);
			static void SetWheelDampingCompression(lua_State *l,SVehicleHandle &hEnt,Float cmp);
			static void SetFrictionSlip(lua_State *l,SVehicleHandle &hEnt,Float slip);
			static void SetSteeringAngle(lua_State *l,SVehicleHandle &hEnt,Float ang);
			static void SetRollInfluence(lua_State *l,SVehicleHandle &hEnt,Float influence);
			static void SetMaxEngineForce(lua_State *l,SVehicleHandle &hEnt,Float force);
			static void SetMaxReverseEngineForce(lua_State *l,SVehicleHandle &hEnt,Float force);
			static void SetMaxBrakeForce(lua_State *l,SVehicleHandle &hEnt,Float force);
			static void SetAcceleration(lua_State *l,SVehicleHandle &hEnt,Float acc);
			static void SetTurnSpeed(lua_State *l,SVehicleHandle &hEnt,Float speed);
			static void SetMaxTurnAngle(lua_State *l,SVehicleHandle &hEnt,Float ang);
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
	def.def("SetWheelDirection",&Lua::Vehicle::Server::SetWheelDirection);
	def.def("SetMaxSuspensionLength",&Lua::Vehicle::Server::SetMaxSuspensionLength);
	def.def("SetMaxSuspensionCompression",&Lua::Vehicle::Server::SetMaxSuspensionCompression);
	def.def("SetWheelRadius",&Lua::Vehicle::Server::SetWheelRadius);
	def.def("SetSuspensionStiffness",&Lua::Vehicle::Server::SetSuspensionStiffness);
	def.def("SetWheelDampingCompression",&Lua::Vehicle::Server::SetWheelDampingCompression);
	def.def("SetFrictionSlip",&Lua::Vehicle::Server::SetFrictionSlip);
	def.def("SetSteeringAngle",&Lua::Vehicle::Server::SetSteeringAngle);
	def.def("SetRollInfluence",&Lua::Vehicle::Server::SetRollInfluence);
	def.def("SetMaxEngineForce",&Lua::Vehicle::Server::SetMaxEngineForce);
	def.def("SetMaxReverseEngineForce",&Lua::Vehicle::Server::SetMaxReverseEngineForce);
	def.def("SetMaxBrakeForce",&Lua::Vehicle::Server::SetMaxBrakeForce);
	def.def("SetAcceleration",&Lua::Vehicle::Server::SetAcceleration);
	def.def("SetTurnSpeed",&Lua::Vehicle::Server::SetTurnSpeed);
	def.def("SetMaxTurnAngle",&Lua::Vehicle::Server::SetMaxTurnAngle);
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
void Lua::Vehicle::Server::SetWheelDirection(lua_State *l,SVehicleHandle &hEnt,const Vector3 &dir)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetWheelDirection(dir);
}
void Lua::Vehicle::Server::SetMaxSuspensionLength(lua_State *l,SVehicleHandle &hEnt,Float len)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxSuspensionLength(len);
}
void Lua::Vehicle::Server::SetMaxSuspensionCompression(lua_State *l,SVehicleHandle &hEnt,Float cmp)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxSuspensionCompression(cmp);
}
void Lua::Vehicle::Server::SetWheelRadius(lua_State *l,SVehicleHandle &hEnt,Float radius)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetWheelRadius(radius);
}
void Lua::Vehicle::Server::SetSuspensionStiffness(lua_State *l,SVehicleHandle &hEnt,Float stiffness)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetSuspensionStiffness(stiffness);
}
void Lua::Vehicle::Server::SetWheelDampingCompression(lua_State *l,SVehicleHandle &hEnt,Float cmp)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetWheelDampingCompression(cmp);
}
void Lua::Vehicle::Server::SetFrictionSlip(lua_State *l,SVehicleHandle &hEnt,Float slip)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetFrictionSlip(slip);
}
void Lua::Vehicle::Server::SetSteeringAngle(lua_State *l,SVehicleHandle &hEnt,Float ang)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetSteeringAngle(ang);
}
void Lua::Vehicle::Server::SetRollInfluence(lua_State *l,SVehicleHandle &hEnt,Float influence)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetRollInfluence(influence);
}
void Lua::Vehicle::Server::SetMaxEngineForce(lua_State *l,SVehicleHandle &hEnt,Float force)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxEngineForce(force);
}
void Lua::Vehicle::Server::SetMaxReverseEngineForce(lua_State *l,SVehicleHandle &hEnt,Float force)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxReverseEngineForce(force);
}
void Lua::Vehicle::Server::SetMaxBrakeForce(lua_State *l,SVehicleHandle &hEnt,Float force)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxBrakeForce(force);
}
void Lua::Vehicle::Server::SetAcceleration(lua_State *l,SVehicleHandle &hEnt,Float acc)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetAcceleration(acc);
}
void Lua::Vehicle::Server::SetTurnSpeed(lua_State *l,SVehicleHandle &hEnt,Float speed)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetTurnSpeed(speed);
}
void Lua::Vehicle::Server::SetMaxTurnAngle(lua_State *l,SVehicleHandle &hEnt,Float ang)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetMaxTurnAngle(ang);
}
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
