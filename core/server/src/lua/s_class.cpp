/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <pragma/lua/converters/file_converter_t.hpp>
#include <pragma/entities/components/liquid/base_liquid_component.hpp>
#include <pragma/model/modelmesh.h>
#include <luainterface.hpp>

#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
void SGame::RegisterLuaClasses()
{
	Game::RegisterLuaClasses();

	auto &modGame = GetLuaInterface().RegisterLibrary("game");
#if 0
	auto &modTestConverters = GetLuaInterface().RegisterLibrary("test_converters");
	modTestConverters[
		luabind::def("test_pair_param",static_cast<void(*)(const std::pair<std::string,float>&)>([](const std::pair<std::string,float> &pair) {
			Con::cout<<"Pair: "<<pair.first<<","<<pair.second<<Con::endl;
		})),
		luabind::def("test_pair_ret",static_cast<std::pair<std::string,float>(*)()>([]() -> std::pair<std::string,float> {
			return {"TestString",1357.f};
		})),

		luabind::def("test_tuple_param",static_cast<void(*)(const std::tuple<std::string,float,Vector3>&)>([](const std::tuple<std::string,float,Vector3> &tuple) {
			Con::cout<<"Tuple: "<<std::get<0>(tuple)<<","<<std::get<1>(tuple)<<","<<std::get<2>(tuple)<<Con::endl;
		})),
		luabind::def("test_tuple_ret",static_cast<std::tuple<std::string,float,Vector3>(*)()>([]() -> std::tuple<std::string,float,Vector3> {
			return {"TestString",1357.f,Vector3{5,87,99}};
		})),

		luabind::def("test_string_view_param",static_cast<void(*)(const std::string_view&)>([](const std::string_view &str) {
			Con::cout<<"string_view: "<<str<<Con::endl;
		})),
		luabind::def("test_string_view_ret",static_cast<std::string_view(*)()>([]() -> std::string_view {
			static std::string str = "TestStringView";
			return str;
		})),

		luabind::def("test_optional_param",static_cast<void(*)(const std::optional<Vector3>&)>([](const std::optional<Vector3> &val) {
			Con::cout<<"optional: ";
			if(!val.has_value())
				Con::cout<<"<no value>";
			else
				Con::cout<<*val;
			Con::cout<<Con::endl;
		})),
		luabind::def("test_optional_ret",static_cast<std::optional<Vector3>(*)(int)>([](int i) -> std::optional<Vector3> {
			if(i == 0)
				return {};
			return Vector3{55,66,77};
		})),

		luabind::def("test_vector_param",static_cast<void(*)(const std::vector<Vector3>&)>([](const std::vector<Vector3> &val) {
			Con::cout<<"values: ";
			for(auto &v : val)
				Con::cout<<v<<Con::endl;
		})),
		luabind::def("test_vector_ret",static_cast<std::vector<Vector3>(*)()>([]() -> std::vector<Vector3> {
			return {Vector3{1,2,3},Vector3{6,5,4},Vector3{8,7,8},Vector3{1,1,1}};
		})),

		luabind::def("test_map_param",static_cast<void(*)(const std::map<std::string,uint32_t>&)>([](const std::map<std::string,uint32_t> &val) {
			Con::cout<<"values: ";
			for(auto &pair : val)
				Con::cout<<pair.first<<" = "<<pair.second<<Con::endl;
		})),
		luabind::def("test_map_ret",static_cast<std::map<std::string,uint32_t>(*)()>([]() -> std::map<std::string,uint32_t> {
			return {
				{"Val_1",5},
				{"Val_2",3},
				{"Val_3",8},
				{"Val_4",55},
				{"Val_5",563}
			};
		})),

		luabind::def("test_array_param",static_cast<void(*)(const std::array<Vector3,3>&)>([](const std::array<Vector3,3> &val) {
			Con::cout<<"values: ";
			for(auto &v : val)
				Con::cout<<v<<Con::endl;
		})),
		luabind::def("test_array_ret",static_cast<std::array<Vector3,3>(*)()>([]() -> std::array<Vector3,3> {
			return {Vector3{1,2,3},Vector3{6,5,4},Vector3{8,7,8}};
		})),

		luabind::def("test_entity_param",static_cast<void(*)(BaseEntity*)>([](BaseEntity *ent) {
			Con::cout<<"Entity: "<<*ent<<Con::endl;
		})),
		luabind::def("test_entity_ret",static_cast<BaseEntity*(*)(Game&)>([](Game &game) -> BaseEntity* {
			return &game.GetWorld()->GetEntity();
		})),

		luabind::def("test_entity_handle_test_get",static_cast<EntityHandleT(*)(Game&)>([](Game &game) -> EntityHandleT {
			auto hTest = game.GetWorld()->GetEntity().GetHandle();
			std::cout<<hTest.get()<<std::endl;
			return hTest;
		})),

		luabind::def("test_entity_handle_param",static_cast<void(*)(EntityHandleT)>([](EntityHandleT ent) {

			//luabind::default_converter<EntityHandleT> x;
			//x.to_cpp(nullptr,luabind::by_reference<EntityHandleT>{},0);

		//	constexpr auto b  = util::is_specialization<EntityHandleT,util::TestHandle>::value;
			//constexpr auto c = luabind::is_type_or_derived<luabind::base_type<typename EntityHandleT::value_type>,BaseEntity>;

			//luabind::default_converter<EntityHandleT> x{};
		//	x.to_cpp(nullptr,luabind::by_value<EntityHandleT>{},0);

			Con::cout<<"Entity: "<<ent->GetClass()<<Con::endl;
		})),
		luabind::def("test_entity_handle_const_param",static_cast<void(*)(const EntityHandleT&)>([](const EntityHandleT &ent) {
			Con::cout<<"Entity: "<<ent->GetClass()<<Con::endl;
		})),
		luabind::def("test_entity_handle_const_pointer_param",static_cast<void(*)(const EntityHandleT*)>([](const EntityHandleT *ent) {
			Con::cout<<"Entity: "<<(*ent)->GetClass()<<Con::endl;
		})),
		luabind::def("test_entity_handle_ret",static_cast<EntityHandleT(*)(Game&)>([](Game &game) -> EntityHandleT {
			return game.GetWorld()->GetEntity().GetHandle();
		})),
		luabind::def("test_entity_handle_const_ret",static_cast<const EntityHandleT(*)(Game&)>([](Game &game) -> const EntityHandleT {
			return game.GetWorld()->GetEntity().GetHandle();
		})),

		luabind::def("test_entity_const_param",static_cast<void(*)(const BaseEntity*)>([](const BaseEntity *ent) {
			Con::cout<<"Entity: "<<const_cast<BaseEntity&>(*ent)<<Con::endl;
		})),
		luabind::def("test_entity_const_ret",static_cast<const BaseEntity*(*)(Game&)>([](Game &game) -> const BaseEntity* {
			return &game.GetWorld()->GetEntity();
		})),

		luabind::def("test_entity_null_param",static_cast<void(*)(BaseEntity*)>([](BaseEntity *ent) {
			Con::cout<<"Entity: "<<ent<<Con::endl;
		})),
		luabind::def("test_entity_null_ret",static_cast<BaseEntity*(*)(Game&)>([](Game &game) -> BaseEntity* {
			return nullptr;
		})),

		luabind::def("test_entity_ref_param",static_cast<void(*)(BaseEntity&)>([](BaseEntity &ent) {
			Con::cout<<"Entity: "<<ent<<Con::endl;
		})),
		luabind::def("test_entity_ref_ret",static_cast<BaseEntity&(*)(Game&)>([](Game &game) -> BaseEntity& {
			return game.GetWorld()->GetEntity();
		})),

		luabind::def("test_entity_ref_const_param",static_cast<void(*)(const BaseEntity&)>([](const BaseEntity &ent) {
			Con::cout<<"Entity: "<<const_cast<BaseEntity&>(ent)<<Con::endl;
		})),
		luabind::def("test_entity_ref_const_ret",static_cast<const BaseEntity&(*)(Game&)>([](Game &game) -> const BaseEntity& {
			return game.GetWorld()->GetEntity();
		})),

		luabind::def("test_component_param",static_cast<void(*)(pragma::VelocityComponent*)>([](pragma::VelocityComponent *c) {
			Con::cout<<"Component: "<<c->GetVelocity()<<Con::endl;
		})),
		luabind::def("test_component_ret",static_cast<pragma::VelocityComponent*(*)(Game&)>([](Game &game) -> pragma::VelocityComponent* {
			EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::VelocityComponent>>();
			auto it = entIt.begin();
			if(it == entIt.end())
				return nullptr;
			return it->GetComponent<pragma::VelocityComponent>().get();
		})),

		luabind::def("test_component_handle_param",static_cast<void(*)(pragma::ComponentHandle<pragma::VelocityComponent>&)>([](pragma::ComponentHandle<pragma::VelocityComponent> &c) {
			Con::cout<<"Component: "<<c->GetVelocity()<<Con::endl;
		})),
		luabind::def("test_component_handle_ret",static_cast<pragma::ComponentHandle<pragma::VelocityComponent>(*)(Game&)>([](Game &game) -> pragma::ComponentHandle<pragma::VelocityComponent> {
			EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::VelocityComponent>>();
			auto it = entIt.begin();
			if(it == entIt.end())
				return pragma::ComponentHandle<pragma::VelocityComponent>{};
			return it->GetComponent<pragma::VelocityComponent>();
		})),
		luabind::def("test_component_handle_null_ret",static_cast<pragma::ComponentHandle<pragma::VelocityComponent>(*)(Game&)>([](Game &game) -> pragma::ComponentHandle<pragma::VelocityComponent> {
			return {};
		})),

		luabind::def("test_hidden_param",static_cast<void(*)(Game&,NetworkState&,Engine&)>([](Game &game,NetworkState &nw,Engine &en) {
			Con::cout<<"Game Map: "<<game.GetMapName()<<Con::endl;
			Con::cout<<"NetworkState material count: "<<nw.GetMaterialManager().GetMaterials().size()<<Con::endl;
			Con::cout<<"Last engine tick: "<<en.GetLastTick()<<Con::endl;
		})),

		luabind::def("test_file",static_cast<void(*)(VFilePtr)>([](VFilePtr f) {
			Con::cout<<"File size: "<<f->GetSize()<<Con::endl;
		})),

		luabind::def("test_property_ret",static_cast<util::PFloatProperty(*)()>([]() {
			//util::PFloatProperty prop;
			//Lua::Property::push(nullptr);
			//luabind::default_converter<util::PFloatProperty> x;
			//constexpr auto b = luabind::is_property<util::PFloatProperty> && !std::is_pointer_v<util::PFloatProperty> && !std::is_reference_v<util::PFloatProperty> && !std::is_const_v<std::remove_reference_t<util::PFloatProperty>>;
			//x.to_cpp();
			return util::FloatProperty::Create(3.f);
		}))
	];
#endif

	auto materialClassDef = luabind::class_<Material>("Material");
	Lua::Material::register_class(materialClassDef);
	materialClassDef.def("SetShader", static_cast<void (*)(lua_State *, ::Material &, const std::string &)>([](lua_State *l, ::Material &mat, const std::string &shader) {
		auto db = mat.GetPropertyDataBlock();
		if(db == nullptr)
			return;
		mat.Initialize(shader, db);
		mat.SetLoaded(true);
	}));
	modGame[materialClassDef];

	auto modelMeshClassDef = luabind::class_<ModelMesh>("Mesh");
	Lua::ModelMesh::register_class(modelMeshClassDef);
	modelMeshClassDef.scope[luabind::def("Create", &Lua::ModelMesh::Server::Create)];

	auto subModelMeshClassDef = luabind::class_<ModelSubMesh>("Sub");
	Lua::ModelSubMesh::register_class(subModelMeshClassDef);
	subModelMeshClassDef.scope[luabind::def("create", &Lua::ModelSubMesh::Server::Create)];

	auto modelClassDef = luabind::class_<Model>("Model");
	Lua::Model::register_class(GetLuaState(), modelClassDef, modelMeshClassDef, subModelMeshClassDef);
	modelClassDef.def("AddMaterial", &Lua::Model::Server::AddMaterial);
	modelClassDef.def("SetMaterial", &Lua::Model::Server::SetMaterial);
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
