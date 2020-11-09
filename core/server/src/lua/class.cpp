/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/classes/lentity.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "pragma/lua/classes/s_lentity.h"
#include "pragma/lua/classes/s_lvhcwheel.h"
#include "pragma/lua/classes/lshaderinfo.h"
#include "pragma/lua/classes/lmaterial.h"
#include "pragma/entities/environment/lights/env_light.h"
#include <pragma/entities/environment/lights/env_light_spot.h>
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/entities/environment/effects/env_particle_system.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/lua/classes/s_lmodel.h"
#include "pragma/ai/ai_task.h"
#include "pragma/lua/classes/s_lai_schedule.h"
#include "pragma/lua/classes/s_lai_behavior.h"
#include <pragma/game/damageinfo.h>
#include "pragma/lua/classes/s_lua_entity.h"
#include "pragma/physics/collision_object.hpp"
#include "pragma/lua/classes/s_lmodelmesh.h"
#include <pragma/entities/func/basefuncwater.h>
#include <pragma/model/modelmesh.h>
#include <luainterface.hpp>

void SGame::RegisterLuaClasses()
{
	Game::RegisterLuaClasses();

	auto &modGame = GetLuaInterface().RegisterLibrary("game");
	auto materialClassDef = luabind::class_<Material>("Material");
	Lua::Material::register_class(materialClassDef);
	materialClassDef.def("SetShader",static_cast<void(*)(lua_State*,::Material&,const std::string&)>([](lua_State *l,::Material &mat,const std::string &shader) {
		auto db = mat.GetDataBlock();
		if(db == nullptr)
			return;
		mat.Initialize(shader,db);
		mat.SetLoaded(true);
	}));
	modGame[materialClassDef];

	auto modelMeshClassDef = luabind::class_<ModelMesh>("Mesh");
	Lua::ModelMesh::register_class(modelMeshClassDef);
	modelMeshClassDef.scope[luabind::def("Create",&Lua::ModelMesh::Server::Create)];

	auto subModelMeshClassDef = luabind::class_<ModelSubMesh>("Sub");
	Lua::ModelSubMesh::register_class(subModelMeshClassDef);
	subModelMeshClassDef.scope[luabind::def("Create",&Lua::ModelSubMesh::Server::Create)];
	subModelMeshClassDef.scope[luabind::def("CreateQuad",&Lua::ModelSubMesh::Server::CreateQuad)];
	subModelMeshClassDef.scope[luabind::def("CreateBox",&Lua::ModelSubMesh::Server::CreateBox)];
	subModelMeshClassDef.scope[luabind::def("CreateSphere",static_cast<void(*)(lua_State*,const Vector3&,float,uint32_t)>(&Lua::ModelSubMesh::Server::CreateSphere))];
	subModelMeshClassDef.scope[luabind::def("CreateSphere",static_cast<void(*)(lua_State*,const Vector3&,float)>(&Lua::ModelSubMesh::Server::CreateSphere))];
	subModelMeshClassDef.scope[luabind::def("CreateCylinder",static_cast<void(*)(lua_State*,float,float,uint32_t)>(&Lua::ModelSubMesh::Server::CreateCylinder))];
	subModelMeshClassDef.scope[luabind::def("CreateCylinder",static_cast<void(*)(lua_State*,float,float)>([](lua_State *l,float startRadius,float length) {
		Lua::ModelSubMesh::Server::CreateCylinder(l,startRadius,length,12);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCone",static_cast<void(*)(lua_State*,float,float,float,uint32_t)>(&Lua::ModelSubMesh::Server::CreateCone))];
	subModelMeshClassDef.scope[luabind::def("CreateCone",static_cast<void(*)(lua_State*,float,float,float)>([](lua_State *l,float startRadius,float length,float endRadius) {
		Lua::ModelSubMesh::Server::CreateCone(l,startRadius,length,endRadius,12);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float,bool,uint32_t)>(&Lua::ModelSubMesh::Server::CreateCircle))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float,bool)>([](lua_State *l,float radius,bool doubleSided) {
		Lua::ModelSubMesh::Server::CreateCircle(l,radius,doubleSided,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float)>([](lua_State *l,float radius) {
		Lua::ModelSubMesh::Server::CreateCircle(l,radius,true,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float,bool,uint32_t)>(&Lua::ModelSubMesh::Server::CreateRing))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float,bool)>([](lua_State *l,float innerRadius,float outerRadius,bool doubleSided) {
		Lua::ModelSubMesh::Server::CreateRing(l,innerRadius,outerRadius,doubleSided,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float)>([](lua_State *l,float innerRadius,float outerRadius) {
		Lua::ModelSubMesh::Server::CreateRing(l,innerRadius,outerRadius,true,36);
	}))];

	auto modelClassDef = luabind::class_<Model>("Model");
	Lua::Model::register_class(GetLuaState(),modelClassDef,modelMeshClassDef,subModelMeshClassDef);
	modelClassDef.def("AddMaterial",&Lua::Model::Server::AddMaterial);
	modelClassDef.def("SetMaterial",&Lua::Model::Server::SetMaterial);
	modGame[modelClassDef];
	auto _G = luabind::globals(GetLuaState());
	_G["Model"] = _G["game"]["Model"];
	_G["Animation"] = _G["game"]["Model"]["Animation"];

	_G["Entity"] = _G["ents"]["Entity"];
	_G["BaseEntity"] = _G["ents"]["BaseEntity"];

	_G["BaseEntityComponent"] = _G["ents"]["BaseEntityComponent"];

	// COMPONENT TODO
	/*auto charClassDef = luabind::class_<pragma::BaseCharacterComponent>("CharacterComponent");
	Lua::BaseCharacter::register_class(charClassDef);
	modGame[charClassDef];

	auto playerClassDef = luabind::class_<PlayerHandle,EntityHandle>("Player");
	Lua::Player::Server::register_class(playerClassDef);
	modGame[playerClassDef];

	auto defWeapon = luabind::class_<WeaponHandle,EntityHandle>("Weapon");
	LUA_CLASS_WEAPON_SHARED(defWeapon);
	modGame[defWeapon];

	auto npcClassDef = luabind::class_<NPCHandle,EntityHandle>("NPC");
	Lua::NPC::Server::register_class(npcClassDef);
	modGame[npcClassDef];

	auto vehicleClassDef = luabind::class_<VehicleHandle,EntityHandle>("Vehicle");
	Lua::Vehicle::Server::register_class(vehicleClassDef);

	auto vhcWheelClassDef = luabind::class_<VHCWheelHandle,EntityHandle>("Wheel")
	LUA_CLASS_VHCWHEEL_SHARED;
	vhcWheelClassDef.def("SetFrontWheel",&Lua_VHCWheel_SetFrontWheel);
	vhcWheelClassDef.def("SetChassisConnectionPoint",&Lua_VHCWheel_SetChassisConnectionPoint);
	vhcWheelClassDef.def("SetWheelAxle",&Lua_VHCWheel_SetWheelAxle);
	vhcWheelClassDef.def("SetWheelDirection",&Lua_VHCWheel_SetWheelDirection);
	vhcWheelClassDef.def("SetMaxSuspensionLength",&Lua_VHCWheel_SetMaxSuspensionLength);
	vhcWheelClassDef.def("SetMaxDampingRelaxation",&Lua_VHCWheel_SetMaxDampingRelaxation);
	vhcWheelClassDef.def("SetMaxSuspensionCompression",&Lua_VHCWheel_SetMaxSuspensionCompression);
	vhcWheelClassDef.def("SetWheelRadius",&Lua_VHCWheel_SetWheelRadius);
	vhcWheelClassDef.def("SetSuspensionStiffness",&Lua_VHCWheel_SetSuspensionStiffness);
	vhcWheelClassDef.def("SetWheelDampingCompression",&Lua_VHCWheel_SetWheelDampingCompression);
	vhcWheelClassDef.def("SetFrictionSlip",&Lua_VHCWheel_SetFrictionSlip);
	vhcWheelClassDef.def("SetSteeringAngle",&Lua_VHCWheel_SetSteeringAngle);
	vhcWheelClassDef.def("SetWheelRotation",&Lua_VHCWheel_SetWheelRotation);
	vhcWheelClassDef.def("SetRollInfluence",&Lua_VHCWheel_SetRollInfluence);
	vehicleClassDef.scope[vhcWheelClassDef];
	modGame[vehicleClassDef];

	// Custom Classes
	auto classDefBase = luabind::class_<SLuaEntityHandle COMMA SLuaEntityWrapper COMMA luabind::bases<EntityHandle>>("BaseEntity");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBase,SLuaEntityWrapper);
	classDefBase.def("SendData",&SLuaEntityWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefBase.def("ReceiveNetEvent",&SLuaEntityWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefBase];*/

	/*auto classDefWeapon = luabind::class_<SLuaWeaponHandle COMMA SLuaWeaponWrapper COMMA luabind::bases<SLuaEntityHandle COMMA WeaponHandle>>("BaseWeapon");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefWeapon,SLuaWeaponWrapper); // TODO Find a way to derive these from BaseEntity directly
	LUA_CUSTOM_CLASS_WEAPON_SHARED(classDefWeapon,SLuaWeaponWrapper);

	classDefWeapon.def("OnPickedUp",&SLuaWeaponWrapper::OnPickedUp,&SLuaWeaponWrapper::default_OnPickedUp);
	//.def("Initialize",&SLuaEntityWrapper::Initialize<SLuaWeaponWrapper>,&SLuaEntityWrapper::default_Initialize)
	//.def("Initialize",&SLuaWeaponWrapper::Initialize,&SLuaWeaponWrapper::default_Initialize)

	classDefWeapon.def("SendData",&SLuaWeaponWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefWeapon.def("ReceiveNetEvent",&SLuaWeaponWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefWeapon];*/

	// COMPONENT TODO
	/*auto classDefBaseVehicle = luabind::class_<SLuaVehicleHandle COMMA SLuaVehicleWrapper COMMA luabind::bases<SLuaEntityHandle COMMA VehicleHandle>>("BaseVehicle");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBaseVehicle,SLuaVehicleWrapper);
	LUA_CUSTOM_CLASS_VEHICLE_SHARED(classDefBaseVehicle,SLuaVehicleWrapper);

	classDefBaseVehicle.def("SendData",&SLuaVehicleWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefBaseVehicle.def("ReceiveNetEvent",&SLuaVehicleWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefBaseVehicle];

	auto classDefBaseNPC = luabind::class_<SLuaNPCHandle COMMA SLuaNPCWrapper COMMA luabind::bases<SLuaEntityHandle COMMA NPCHandle>>("BaseNPC");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBaseNPC,SLuaNPCWrapper);
	LUA_CUSTOM_CLASS_NPC_SHARED(classDefBaseNPC,SLuaNPCWrapper);

	classDefBaseNPC.def("SendData",&SLuaNPCWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefBaseNPC.def("ReceiveNetEvent",&SLuaNPCWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	classDefBaseNPC.def("SelectSchedule",&SLuaNPCWrapper::SelectSchedule,&SLuaNPCWrapper::default_SelectSchedule);
	classDefBaseNPC.def("OnPrimaryTargetChanged",&SLuaNPCWrapper::OnPrimaryTargetChanged,&SLuaNPCWrapper::default_OnPrimaryTargetChanged);
	classDefBaseNPC.def("OnNPCStateChanged",&SLuaNPCWrapper::OnNPCStateChanged,&SLuaNPCWrapper::default_OnNPCStateChanged);
	classDefBaseNPC.def("OnTargetVisibilityLost",&SLuaNPCWrapper::OnTargetVisibilityLost,&SLuaNPCWrapper::default_OnTargetVisibilityLost);
	classDefBaseNPC.def("OnTargetVisibilityReacquired",&SLuaNPCWrapper::OnTargetVisibilityReacquired,&SLuaNPCWrapper::default_OnTargetVisibilityReacquired);
	classDefBaseNPC.def("OnMemoryGained",&SLuaNPCWrapper::OnMemoryGained,&SLuaNPCWrapper::default_OnMemoryGained);
	classDefBaseNPC.def("OnMemoryLost",&SLuaNPCWrapper::OnMemoryLost,&SLuaNPCWrapper::default_OnMemoryLost);
	classDefBaseNPC.def("OnTargetAcquired",&SLuaNPCWrapper::OnTargetAcquired,&SLuaNPCWrapper::default_OnTargetAcquired);
	classDefBaseNPC.def("OnScheduleComplete",&SLuaNPCWrapper::OnScheduleComplete,&SLuaNPCWrapper::default_OnScheduleComplete);
	classDefBaseNPC.def("OnSuspiciousSoundHeared",&SLuaNPCWrapper::OnSuspiciousSoundHeared,&SLuaNPCWrapper::default_OnSuspiciousSoundHeared);
	classDefBaseNPC.def("OnControllerActionInput",&SLuaNPCWrapper::OnControllerActionInput,&SLuaNPCWrapper::default_OnControllerActionInput);
	classDefBaseNPC.def("OnStartControl",&SLuaNPCWrapper::OnStartControl,&SLuaNPCWrapper::default_OnStartControl);
	classDefBaseNPC.def("OnEndControl",&SLuaNPCWrapper::OnEndControl,&SLuaNPCWrapper::default_OnEndControl);
	classDefBaseNPC.def("OnDeath",&SLuaNPCWrapper::OnDeath,&SLuaNPCWrapper::default_OnDeath);
	classDefBaseNPC.def("PlayFootStepSound",&SLuaNPCWrapper::PlayFootStepSound,&SLuaNPCWrapper::default_PlayFootStepSound);
	classDefBaseNPC.def("CalcMovementSpeed",&SLuaNPCWrapper::CalcMovementSpeed,&SLuaNPCWrapper::default_CalcMovementSpeed);
	modGame[classDefBaseNPC];
	//
	
	auto defEnvParticleSystem = luabind::class_<EnvParticleSystemHandle,EntityHandle>("EnvParticleSystem");
	modGame[defEnvParticleSystem];

	auto envLightClassDef = luabind::class_<EnvLightHandle,EntityHandle>("EnvLight");
	Lua::EnvLight::register_class(envLightClassDef);
	modGame[envLightClassDef];

	auto envLightSpotClassDef = luabind::class_<EnvLightSpotHandle COMMA EnvLightHandle,EntityHandle>("EnvLightSpot");
	Lua::EnvLightSpot::register_class(envLightSpotClassDef);
	modGame[envLightSpotClassDef];

	auto envLightPointClassDef = luabind::class_<EnvLightPointHandle COMMA EnvLightHandle,EntityHandle>("EnvLightPoint");
	Lua::EnvLightPoint::register_class(envLightPointClassDef);
	modGame[envLightPointClassDef];

	auto envLightDirectionalClassDef = luabind::class_<EnvLightDirectionalHandle COMMA EnvLightHandle,EntityHandle>("EnvLightDirectional");
	Lua::EnvLightDirectional::register_class(envLightDirectionalClassDef);
	modGame[envLightDirectionalClassDef];

	auto funcWaterClassDef = luabind::class_<FuncWaterHandle,EntityHandle>("FuncWater");
	Lua::FuncWater::register_class(funcWaterClassDef);
	modGame[funcWaterClassDef];

	_G["Entity"] = _G["game"]["Entity"];
	_G["BaseEntity"] = _G["game"]["BaseEntity"];
	_G["BaseWeapon"] = _G["game"]["BaseWeapon"];
	_G["BaseVehicle"] = _G["game"]["BaseVehicle"];
	_G["BaseNPC"] = _G["game"]["BaseNPC"];

	auto &modShader = GetLuaInterface().RegisterLibrary("shader");
	auto defShaderInfo = luabind::class_<ShaderInfo>("Info");
	defShaderInfo.def("GetName",&Lua_ShaderInfo_GetName);
	modShader[defShaderInfo];*/
	/*lua_bind(
		luabind::class_<AITask>("AITask")
		.def("AddParameter",(void(AITask::*)(bool))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(float))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(std::string))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(BaseEntity*))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(EntityHandle&))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(Vector3&))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(Quat&))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(EulerAngles&))&AITask::AddParameter)
	);*/ // TODO
}