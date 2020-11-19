/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include <pragma/lua/classes/lproperty_generic.hpp>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_angle.h>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/model/model.h>
#include <pragma/physics/raytraces.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>

namespace Lua
{
	namespace PBRConverter
	{
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,Model &mdl,uint32_t width,uint32_t height,uint32_t samples,bool rebuild)
		{
			hComponent->GenerateAmbientOcclusionMaps(mdl,width,height,samples,rebuild);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,Model &mdl,uint32_t width,uint32_t height,uint32_t samples)
		{
			hComponent->GenerateAmbientOcclusionMaps(mdl,width,height,samples);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,Model &mdl,uint32_t width,uint32_t height)
		{
			hComponent->GenerateAmbientOcclusionMaps(mdl,width,height);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,Model &mdl)
		{
			hComponent->GenerateAmbientOcclusionMaps(mdl);
		}
		
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,EntityHandle &hEnt,uint32_t width,uint32_t height,uint32_t samples,bool rebuild)
		{
			LUA_CHECK_ENTITY(l,hEnt);
			hComponent->GenerateAmbientOcclusionMaps(*hEnt.get(),width,height,samples,rebuild);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,EntityHandle &hEnt,uint32_t width,uint32_t height,uint32_t samples)
		{
			LUA_CHECK_ENTITY(l,hEnt);
			hComponent->GenerateAmbientOcclusionMaps(*hEnt.get(),width,height,samples);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,EntityHandle &hEnt,uint32_t width,uint32_t height)
		{
			LUA_CHECK_ENTITY(l,hEnt);
			hComponent->GenerateAmbientOcclusionMaps(*hEnt.get(),width,height);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,CPBRConverterHandle &hComponent,EntityHandle &hEnt)
		{
			LUA_CHECK_ENTITY(l,hEnt);
			hComponent->GenerateAmbientOcclusionMaps(*hEnt.get());
		}
	};
	namespace ParticleSystem
	{
		static std::string get_key_value(lua_State *l,int32_t argIdx)
		{
			if(Lua::IsNumber(l,argIdx))
				return std::to_string(Lua::CheckNumber(l,argIdx));
			if(Lua::IsBool(l,argIdx))
				return Lua::CheckBool(l,argIdx) ? "1" : "0";
			if(Lua::IsVector4(l,argIdx))
			{
				auto &v = *Lua::CheckVector4(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z) +" " +std::to_string(v.w);
			}
			if(Lua::IsVector(l,argIdx))
			{
				auto &v = *Lua::CheckVector(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z);
			}
			if(Lua::IsVector2(l,argIdx))
			{
				auto &v = *Lua::CheckVector2(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y);
			}
			if(Lua::IsVector4i(l,argIdx))
			{
				auto &v = *Lua::CheckVector4i(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z) +" " +std::to_string(v.w);
			}
			if(Lua::IsVectori(l,argIdx))
			{
				auto &v = *Lua::CheckVectori(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z);
			}
			if(Lua::IsVector2i(l,argIdx))
			{
				auto &v = *Lua::CheckVector2i(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y);
			}
			if(Lua::IsColor(l,argIdx))
			{
				auto &c = *Lua::CheckColor(l,argIdx);
				return std::to_string(c.r) +" " +std::to_string(c.g) +" " +std::to_string(c.b) +" " +std::to_string(c.a);
			}
			if(Lua::IsEulerAngles(l,argIdx))
			{
				auto &ang = *Lua::CheckEulerAngles(l,argIdx);
				return std::to_string(ang.p) +" " +std::to_string(ang.y) +" " +std::to_string(ang.r);
			}
			if(Lua::IsQuaternion(l,argIdx))
			{
				auto &rot = *Lua::CheckQuaternion(l,argIdx);
				return std::to_string(rot.w) +" " +std::to_string(rot.x) +" " +std::to_string(rot.y) +" " +std::to_string(rot.z);
			}
			return Lua::CheckString(l,argIdx);
		}
	};
	namespace Decal
	{
		static void create_from_projection(lua_State *l,CDecalHandle &hComponent,luabind::object tMeshes,const umath::ScaledTransform &pose)
		{
			pragma::Lua::check_component(l,hComponent);
			int32_t t = 2;
			Lua::CheckTable(l,t);
			std::vector<pragma::DecalProjector::MeshData> meshesDatas {};
			auto numMeshes = Lua::GetObjectLength(l,t);
			meshesDatas.reserve(numMeshes);
			for(auto i=decltype(numMeshes){0u};i<numMeshes;++i)
			{
				meshesDatas.push_back({});
				auto &meshData = meshesDatas.back();
				Lua::PushInt(l,i +1); /* 1 */
				Lua::GetTableValue(l,t); /* 1 */

				auto tMeshData = Lua::GetStackTop(l);
				Lua::CheckTable(l,tMeshData);

				Lua::PushString(l,"pose"); /* 2 */
				Lua::GetTableValue(l,tMeshData); /* 2 */
				meshData.pose = Lua::Check<umath::ScaledTransform>(l,-1);
				Lua::Pop(l,1); /* 1 */
				
				Lua::PushString(l,"subMeshes"); /* 2 */
				Lua::GetTableValue(l,tMeshData); /* 2 */
				auto tSubMeshes = Lua::GetStackTop(l);
				Lua::CheckTable(l,tSubMeshes);
				auto numSubMeshes = Lua::GetObjectLength(l,tSubMeshes);
				meshData.subMeshes.reserve(numSubMeshes);
				for(auto j=decltype(numSubMeshes){0u};j<numSubMeshes;++j)
				{
					Lua::PushInt(l,j +1); /* 1 */
					Lua::GetTableValue(l,tSubMeshes); /* 1 */
					auto &mesh = Lua::Check<ModelSubMesh>(l,-1);
					meshData.subMeshes.push_back(&mesh);
					Lua::Pop(l,1); /* 0 */
				}
				Lua::Pop(l,1); /* 1 */

				Lua::Pop(l,1); /* 0 */
			}
			Lua::PushBool(l,hComponent->ApplyDecal(meshesDatas));
		}
		static void create_from_projection(lua_State *l,CDecalHandle &hComponent,luabind::object tMeshes)
		{
			create_from_projection(l,hComponent,tMeshes,{});
		}
	};
};

void CGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);
	auto *l = GetLuaState();
	Lua::register_cl_ai_component(l,entsMod);
	Lua::register_cl_character_component(l,entsMod);
	Lua::register_cl_player_component(l,entsMod);
	Lua::register_cl_vehicle_component(l,entsMod);
	Lua::register_cl_weapon_component(l,entsMod);

	auto defCColor = luabind::class_<CColorHandle,BaseEntityComponentHandle>("ColorComponent");
	Lua::register_base_color_component_methods<luabind::class_<CColorHandle,BaseEntityComponentHandle>,CColorHandle>(l,defCColor);
	entsMod[defCColor];

	auto defCScore = luabind::class_<CScoreHandle,BaseEntityComponentHandle>("ScoreComponent");
	Lua::register_base_score_component_methods<luabind::class_<CScoreHandle,BaseEntityComponentHandle>,CScoreHandle>(l,defCScore);
	entsMod[defCScore];

	auto defCFlammable = luabind::class_<CFlammableHandle,BaseEntityComponentHandle>("FlammableComponent");
	Lua::register_base_flammable_component_methods<luabind::class_<CFlammableHandle,BaseEntityComponentHandle>,CFlammableHandle>(l,defCFlammable);
	entsMod[defCFlammable];

	auto defCHealth = luabind::class_<CHealthHandle,BaseEntityComponentHandle>("HealthComponent");
	Lua::register_base_health_component_methods<luabind::class_<CHealthHandle,BaseEntityComponentHandle>,CHealthHandle>(l,defCHealth);
	entsMod[defCHealth];

	auto defCName = luabind::class_<CNameHandle,BaseEntityComponentHandle>("NameComponent");
	Lua::register_base_name_component_methods<luabind::class_<CNameHandle,BaseEntityComponentHandle>,CNameHandle>(l,defCName);
	entsMod[defCName];

	auto defCNetworked = luabind::class_<CNetworkedHandle,BaseEntityComponentHandle>("NetworkedComponent");
	Lua::register_base_networked_component_methods<luabind::class_<CNetworkedHandle,BaseEntityComponentHandle>,CNetworkedHandle>(l,defCNetworked);
	entsMod[defCNetworked];

	auto defCObservable = luabind::class_<CObservableHandle,BaseEntityComponentHandle>("ObservableComponent");
	Lua::register_base_observable_component_methods<luabind::class_<CObservableHandle,BaseEntityComponentHandle>,CObservableHandle>(l,defCObservable);
	entsMod[defCObservable];

	auto defCShooter = luabind::class_<CShooterHandle,BaseEntityComponentHandle>("ShooterComponent");
	Lua::register_base_shooter_component_methods<luabind::class_<CShooterHandle,BaseEntityComponentHandle>,CShooterHandle>(l,defCShooter);
	entsMod[defCShooter];

	auto defCPhysics = luabind::class_<CPhysicsHandle,BaseEntityComponentHandle>("PhysicsComponent");
	Lua::register_base_physics_component_methods<luabind::class_<CPhysicsHandle,BaseEntityComponentHandle>,CPhysicsHandle>(l,defCPhysics);
	entsMod[defCPhysics];

	auto defCRadius = luabind::class_<CRadiusHandle,BaseEntityComponentHandle>("RadiusComponent");
	Lua::register_base_radius_component_methods<luabind::class_<CRadiusHandle,BaseEntityComponentHandle>,CRadiusHandle>(l,defCRadius);
	entsMod[defCRadius];

	auto defCWorld = luabind::class_<CWorldHandle,BaseEntityComponentHandle>("WorldComponent");
	Lua::register_base_world_component_methods<luabind::class_<CWorldHandle,BaseEntityComponentHandle>,CWorldHandle>(l,defCWorld);
	defCWorld.def("GetBSPTree",static_cast<void(*)(lua_State*,CWorldHandle&)>([](lua_State *l,CWorldHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		auto bspTree = hEnt->GetBSPTree();
		if(bspTree == nullptr)
			return;
		Lua::Push(l,bspTree);
	}));

	auto defCEye = luabind::class_<CEyeHandle,BaseEntityComponentHandle>("EyeComponent");
	defCEye.def("GetViewTarget",static_cast<void(*)(lua_State*,CEyeHandle&)>([](lua_State *l,CEyeHandle &hEye) {
		pragma::Lua::check_component(l,hEye);
		Lua::Push<Vector3>(l,hEye->GetViewTarget());
		}));
	defCEye.def("SetViewTarget",static_cast<void(*)(lua_State*,CEyeHandle&,const Vector3&)>([](lua_State *l,CEyeHandle &hEye,const Vector3 &viewTarget) {
		pragma::Lua::check_component(l,hEye);
		hEye->SetViewTarget(viewTarget);
		}));
	defCEye.def("ClearViewTarget",static_cast<void(*)(lua_State*,CEyeHandle&)>([](lua_State *l,CEyeHandle &hEye) {
		pragma::Lua::check_component(l,hEye);
		hEye->ClearViewTarget();
	}));
	defCEye.def("GetEyeShift",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::Push<Vector3>(l,config->eyeShift);
		}));
	defCEye.def("SetEyeShift",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t,const Vector3&)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex,const Vector3 &eyeShift) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeShift = eyeShift;
		}));
	defCEye.def("GetEyeJitter",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::Push<Vector2>(l,config->jitter);
		}));
	defCEye.def("SetEyeJitter",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t,const Vector2&)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex,const Vector2 &eyeJitter) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->jitter = eyeJitter;
		}));
	defCEye.def("GetEyeSize",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::PushNumber(l,config->eyeSize);
		}));
	defCEye.def("SetEyeSize",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t,float)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex,float eyeSize) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeSize = eyeSize;
		}));
	defCEye.def("SetIrisDilation",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t,float)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex,float dilation) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->dilation = dilation;
		}));
	defCEye.def("GetIrisDilation",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex) {
		pragma::Lua::check_component(l,hEye);
		auto *config = hEye->GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::PushNumber(l,config->dilation);
		}));
	defCEye.def("CalcEyeballPose",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex) {
		pragma::Lua::check_component(l,hEye);
		auto pose = hEye->CalcEyeballPose(eyeIndex);
		Lua::Push(l,pose);
		}));
	defCEye.def("GetEyeballState",static_cast<void(*)(lua_State*,CEyeHandle&,uint32_t)>([](lua_State *l,CEyeHandle &hEye,uint32_t eyeIndex) {
		pragma::Lua::check_component(l,hEye);
		auto *eyeballData = hEye->GetEyeballData(eyeIndex);
		if(eyeballData == nullptr)
			return;
		auto &eyeballState = eyeballData->state;
		Lua::Push<pragma::CEyeComponent::EyeballState*>(l,&eyeballState);
		}));
	defCEye.def("SetBlinkDuration",static_cast<void(*)(lua_State*,CEyeHandle&,float)>([](lua_State *l,CEyeHandle &hEye,float blinkDuration) {
		pragma::Lua::check_component(l,hEye);
		hEye->SetBlinkDuration(blinkDuration);
	}));
	defCEye.def("GetBlinkDuration",static_cast<void(*)(lua_State*,CEyeHandle&)>([](lua_State *l,CEyeHandle &hEye) {
		pragma::Lua::check_component(l,hEye);
		Lua::PushNumber(l,hEye->GetBlinkDuration());
	}));
	defCEye.def("SetBlinkingEnabled",static_cast<void(*)(lua_State*,CEyeHandle&,bool)>([](lua_State *l,CEyeHandle &hEye,bool blinkingEnabled) {
		pragma::Lua::check_component(l,hEye);
		hEye->SetBlinkingEnabled(blinkingEnabled);
	}));
	defCEye.def("IsBlinkingEnabled",static_cast<void(*)(lua_State*,CEyeHandle&)>([](lua_State *l,CEyeHandle &hEye) {
		pragma::Lua::check_component(l,hEye);
		Lua::PushBool(l,hEye->IsBlinkingEnabled());
	}));

	auto defEyeballState = luabind::class_<pragma::CEyeComponent::EyeballState>("EyeballState");
	defEyeballState.def_readwrite("origin",&pragma::CEyeComponent::EyeballState::origin);
	defEyeballState.def_readwrite("forward",&pragma::CEyeComponent::EyeballState::forward);
	defEyeballState.def_readwrite("right",&pragma::CEyeComponent::EyeballState::right);
	defEyeballState.def_readwrite("up",&pragma::CEyeComponent::EyeballState::up);
	defEyeballState.def_readwrite("irisProjectionU",&pragma::CEyeComponent::EyeballState::irisProjectionU);
	defEyeballState.def_readwrite("irisProjectionV",&pragma::CEyeComponent::EyeballState::irisProjectionV);
	defCEye.scope[defEyeballState];

	// defCEye.add_static_constant("EVENT_ON_EYEBALLS_UPDATED",pragma::CEyeComponent::EVENT_ON_EYEBALLS_UPDATED);
	// defCEye.add_static_constant("EVENT_ON_BLINK",pragma::CEyeComponent::EVENT_ON_BLINK);
	entsMod[defCEye];

	auto defCScene = luabind::class_<CSceneHandle,BaseEntityComponentHandle>("SceneComponent");
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BRUTE_FORCE",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::BruteForce));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_CHC_PLUSPLUS",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::CHCPP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BSP",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::BSP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_OCTREE",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::Octree));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_INERT",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::Inert));
	defCScene.add_static_constant("EVENT_ON_ACTIVE_CAMERA_CHANGED",pragma::CSceneComponent::CSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED);
	defCScene.add_static_constant("DEBUG_MODE_NONE",umath::to_integral(pragma::SceneDebugMode::None));
	defCScene.add_static_constant("DEBUG_MODE_AMBIENT_OCCLUSION",umath::to_integral(pragma::SceneDebugMode::AmbientOcclusion));
	defCScene.add_static_constant("DEBUG_MODE_ALBEDO",umath::to_integral(pragma::SceneDebugMode::Albedo));
	defCScene.add_static_constant("DEBUG_MODE_METALNESS",umath::to_integral(pragma::SceneDebugMode::Metalness));
	defCScene.add_static_constant("DEBUG_MODE_ROUGHNESS",umath::to_integral(pragma::SceneDebugMode::Roughness));
	defCScene.add_static_constant("DEBUG_MODE_DIFFUSE_LIGHTING",umath::to_integral(pragma::SceneDebugMode::DiffuseLighting));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL",umath::to_integral(pragma::SceneDebugMode::Normal));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL_MAP",umath::to_integral(pragma::SceneDebugMode::NormalMap));
	defCScene.add_static_constant("DEBUG_MODE_REFLECTANCE",umath::to_integral(pragma::SceneDebugMode::Reflectance));
	defCScene.add_static_constant("DEBUG_MODE_IBL_PREFILTER",umath::to_integral(pragma::SceneDebugMode::IBLPrefilter));
	defCScene.add_static_constant("DEBUG_MODE_IBL_IRRADIANCE",umath::to_integral(pragma::SceneDebugMode::IBLIrradiance));
	defCScene.add_static_constant("DEBUG_MODE_EMISSION",umath::to_integral(pragma::SceneDebugMode::Emission));
	defCScene.def("GetActiveCamera",&Lua::Scene::GetCamera);
	defCScene.def("SetActiveCamera",static_cast<void(*)(lua_State*,CSceneHandle&,CCameraHandle&)>([](lua_State *l,CSceneHandle &scene,CCameraHandle &hCam) {
		pragma::Lua::check_component(l,scene);
		pragma::Lua::check_component(l,hCam);
		scene->SetActiveCamera(*hCam);
	}));
	defCScene.def("SetOcclusionCullingMethod",static_cast<void(*)(lua_State*,CSceneHandle&,uint32_t)>([](lua_State *l,CSceneHandle &scene,uint32_t method) {
		pragma::Lua::check_component(l,scene);
		scene->GetSceneRenderDesc().SetOcclusionCullingMethod(static_cast<SceneRenderDesc::OcclusionCullingMethod>(method));
	}));
	defCScene.def("GetWidth",&Lua::Scene::GetWidth);
	defCScene.def("GetHeight",&Lua::Scene::GetHeight);
	defCScene.def("GetSize",&Lua::Scene::GetSize);
	defCScene.def("Resize",&Lua::Scene::Resize);
	defCScene.def("BeginDraw",&Lua::Scene::BeginDraw);
	defCScene.def("UpdateBuffers",&Lua::Scene::UpdateBuffers);
	defCScene.def("GetWorldEnvironment",&Lua::Scene::GetWorldEnvironment);
	defCScene.def("SetWorldEnvironment",&Lua::Scene::SetWorldEnvironment);
	defCScene.def("ClearWorldEnvironment",&Lua::Scene::ClearWorldEnvironment);
	defCScene.def("InitializeRenderTarget",&Lua::Scene::ReloadRenderTarget);

	defCScene.def("GetIndex",&Lua::Scene::GetIndex);
	defCScene.def("GetCameraDescriptorSet",static_cast<void(*)(lua_State*,CSceneHandle&,uint32_t)>(&Lua::Scene::GetCameraDescriptorSet));
	defCScene.def("GetCameraDescriptorSet",static_cast<void(*)(lua_State*,CSceneHandle&)>(&Lua::Scene::GetCameraDescriptorSet));
	defCScene.def("GetViewCameraDescriptorSet",&Lua::Scene::GetViewCameraDescriptorSet);
	defCScene.def("GetDebugMode",&Lua::Scene::GetDebugMode);
	defCScene.def("SetDebugMode",&Lua::Scene::SetDebugMode);
	defCScene.def("Link",&Lua::Scene::Link);
	defCScene.def("BuildRenderQueue",&Lua::Scene::BuildRenderQueue);
	defCScene.def("RenderPrepass",&Lua::Scene::RenderPrepass);
	defCScene.def("Render",static_cast<void(*)(lua_State*,CSceneHandle&,::util::DrawSceneInfo&,RenderMode,RenderFlags)>(Lua::Scene::Render));
	defCScene.def("Render",static_cast<void(*)(lua_State*,CSceneHandle&,::util::DrawSceneInfo&,RenderMode)>(Lua::Scene::Render));
	defCScene.def("GetRenderer",static_cast<void(*)(lua_State*,CSceneHandle&)>([](lua_State *l,CSceneHandle &scene) {
		pragma::Lua::check_component(l,scene);
		auto *renderer = scene->GetRenderer();
		if(renderer == nullptr)
			return;
		Lua::Push<std::shared_ptr<pragma::rendering::BaseRenderer>>(l,renderer->shared_from_this());
	}));
	defCScene.def("SetRenderer",static_cast<void(*)(lua_State*,CSceneHandle&,pragma::rendering::BaseRenderer&)>([](lua_State *l,CSceneHandle &scene,pragma::rendering::BaseRenderer &renderer) {
		pragma::Lua::check_component(l,scene);
		scene->SetRenderer(renderer.shared_from_this());
	}));
	enum class RendererType : uint32_t
	{
		Rasterization = 0u,
		Raytracing
	};
	defCScene.def("CreateRenderer",static_cast<void(*)(lua_State*,CSceneHandle&,uint32_t)>([](lua_State *l,CSceneHandle &scene,uint32_t type) {
		pragma::Lua::check_component(l,scene);
		switch(static_cast<RendererType>(type))
		{
		case RendererType::Rasterization:
		{
			auto renderer = pragma::rendering::BaseRenderer::Create<pragma::rendering::RasterizationRenderer>(scene->GetWidth(),scene->GetHeight());
			if(renderer == nullptr)
				return;
			Lua::Push(l,renderer);
			break;
		}
		case RendererType::Raytracing:
		{
			auto renderer = pragma::rendering::BaseRenderer::Create<pragma::rendering::RaytracingRenderer>(scene->GetWidth(),scene->GetHeight());
			if(renderer == nullptr)
				return;
			Lua::Push(l,renderer);
			break;
		}
		}
	}));
	defCScene.def("SetRenderer",static_cast<void(*)(lua_State*,CSceneHandle&,uint32_t)>([](lua_State *l,CSceneHandle &scene,uint32_t type) {
		pragma::Lua::check_component(l,scene);
		switch(static_cast<RendererType>(type))
		{
		case RendererType::Rasterization:
		{
			auto renderer = pragma::rendering::BaseRenderer::Create<pragma::rendering::RasterizationRenderer>(scene->GetWidth(),scene->GetHeight());
			if(renderer == nullptr)
				return;
			scene->SetRenderer(renderer);
			Lua::Push(l,renderer);
			break;
		}
		case RendererType::Raytracing:
		{
			auto renderer = pragma::rendering::BaseRenderer::Create<pragma::rendering::RaytracingRenderer>(scene->GetWidth(),scene->GetHeight());
			if(renderer == nullptr)
				return;
			scene->SetRenderer(renderer);
			Lua::Push(l,renderer);
			break;
		}
		}
	}));
	defCScene.def("GetSceneIndex",static_cast<void(*)(lua_State*,CSceneHandle&)>([](lua_State *l,CSceneHandle &scene) {
		pragma::Lua::check_component(l,scene);
		Lua::PushInt(l,scene->GetSceneIndex());
	}));
	defCScene.def("SetParticleSystemColorFactor",static_cast<void(*)(lua_State*,CSceneHandle&,const Vector4&)>([](lua_State *l,CSceneHandle &scene,const Vector4 &factor) {
		pragma::Lua::check_component(l,scene);
		scene->SetParticleSystemColorFactor(factor);
	}));
	defCScene.def("GetParticleSystemColorFactor",static_cast<void(*)(lua_State*,CSceneHandle&)>([](lua_State *l,CSceneHandle &scene) {
		pragma::Lua::check_component(l,scene);
		Lua::Push<Vector4>(l,scene->GetParticleSystemColorFactor());
	}));
	defCScene.def("GetRenderParticleSystems",static_cast<void(*)(lua_State*,CSceneHandle&)>([](lua_State *l,CSceneHandle &scene) {
		pragma::Lua::check_component(l,scene);
		auto &particleSystems = scene->GetSceneRenderDesc().GetCulledParticles();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &pts : particleSystems)
		{
			if(pts == nullptr)
				continue;
			Lua::PushInt(l,idx++);
			pts->PushLuaObject(l);
			Lua::SetTableValue(l,t);
		}
	}));

	// Texture indices for scene render target
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_COLOR",0u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_BLOOM",1u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_DEPTH",2u);

	defCScene.add_static_constant("RENDERER_TYPE_RASTERIZATION",umath::to_integral(RendererType::Rasterization));
	defCScene.add_static_constant("RENDERER_TYPE_RAYTRACING",umath::to_integral(RendererType::Raytracing));
	entsMod[defCScene];

	Lua::Render::register_class(l,entsMod);
	Lua::ModelDef::register_class(l,entsMod);
	Lua::Animated::register_class(l,entsMod);
	Lua::Flex::register_class(l,entsMod);
	Lua::BSP::register_class(l,entsMod,defCWorld);
	Lua::Lightmap::register_class(l,entsMod);
	Lua::VertexAnimated::register_class(l,entsMod);
	Lua::SoundEmitter::register_class(l,entsMod);
	entsMod[defCWorld];

	auto &componentManager = engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager();

	auto defCToggle = luabind::class_<CToggleHandle,BaseEntityComponentHandle>("ToggleComponent");
	Lua::register_base_toggle_component_methods<luabind::class_<CToggleHandle,BaseEntityComponentHandle>,CToggleHandle>(l,defCToggle);
	entsMod[defCToggle];

	auto defCTransform = luabind::class_<CTransformHandle,BaseEntityComponentHandle>("TransformComponent");
	Lua::register_base_transform_component_methods<luabind::class_<CTransformHandle,BaseEntityComponentHandle>,CTransformHandle>(l,defCTransform);
	entsMod[defCTransform];

	auto defCWheel = luabind::class_<CWheelHandle,BaseEntityComponentHandle>("WheelComponent");
	Lua::register_base_wheel_component_methods<luabind::class_<CWheelHandle,BaseEntityComponentHandle>,CWheelHandle>(l,defCWheel);
	entsMod[defCWheel];

	auto defCSoundDsp = luabind::class_<CSoundDspHandle,BaseEntityComponentHandle>("SoundDspComponent");
	Lua::register_base_env_sound_dsp_component_methods<luabind::class_<CSoundDspHandle,BaseEntityComponentHandle>,CSoundDspHandle>(l,defCSoundDsp);
	entsMod[defCSoundDsp];

	auto defCSoundDspChorus = luabind::class_<CSoundDspChorusHandle,BaseEntityComponentHandle>("SoundDspChorusComponent");
	entsMod[defCSoundDspChorus];

	auto defCSoundDspDistortion = luabind::class_<CSoundDspDistortionHandle,BaseEntityComponentHandle>("SoundDspDistortionComponent");
	entsMod[defCSoundDspDistortion];

	auto defCSoundDspEAXReverb = luabind::class_<CSoundDspEAXReverbHandle,BaseEntityComponentHandle>("SoundDspEAXReverbComponent");
	entsMod[defCSoundDspEAXReverb];

	auto defCSoundDspEcho = luabind::class_<CSoundDspEchoHandle,BaseEntityComponentHandle>("SoundDspEchoComponent");
	entsMod[defCSoundDspEcho];

	auto defCSoundDspEqualizer = luabind::class_<CSoundDspEqualizerHandle,BaseEntityComponentHandle>("SoundDspEqualizerComponent");
	entsMod[defCSoundDspEqualizer];

	auto defCSoundDspFlanger = luabind::class_<CSoundDspFlangerHandle,BaseEntityComponentHandle>("SoundDspFlangerComponent");
	entsMod[defCSoundDspFlanger];

	auto defCCamera = luabind::class_<CCameraHandle,BaseEntityComponentHandle>("CameraComponent");
	Lua::register_base_env_camera_component_methods<luabind::class_<CCameraHandle,BaseEntityComponentHandle>,CCameraHandle>(l,defCCamera);
	entsMod[defCCamera];

	auto defCDecal = luabind::class_<CDecalHandle,BaseEntityComponentHandle>("DecalComponent");
	Lua::register_base_decal_component_methods<luabind::class_<CDecalHandle,BaseEntityComponentHandle>,CDecalHandle>(l,defCDecal);
	defCDecal.def("CreateFromProjection",static_cast<void(*)(lua_State*,CDecalHandle&,luabind::object,const umath::ScaledTransform&)>(&Lua::Decal::create_from_projection));
	defCDecal.def("CreateFromProjection",static_cast<void(*)(lua_State*,CDecalHandle&,luabind::object)>(&Lua::Decal::create_from_projection));
	defCDecal.def("DebugDraw",static_cast<void(*)(lua_State*,CDecalHandle&,float)>([](lua_State *l,CDecalHandle &hEnt,float duration) {
		pragma::Lua::check_component(l,hEnt);
		hEnt->GetProjector().DebugDraw(duration);
	}));
	entsMod[defCDecal];

	auto defCExplosion = luabind::class_<CExplosionHandle,BaseEntityComponentHandle>("ExplosionComponent");
	Lua::register_base_env_explosion_component_methods<luabind::class_<CExplosionHandle,BaseEntityComponentHandle>,CExplosionHandle>(l,defCExplosion);
	entsMod[defCExplosion];

	auto defCFire = luabind::class_<CFireHandle,BaseEntityComponentHandle>("FireComponent");
	Lua::register_base_env_fire_component_methods<luabind::class_<CFireHandle,BaseEntityComponentHandle>,CFireHandle>(l,defCFire);
	entsMod[defCFire];

	auto defCFogController = luabind::class_<CFogControllerHandle,BaseEntityComponentHandle>("FogControllerComponent");
	Lua::register_base_env_fog_controller_component_methods<luabind::class_<CFogControllerHandle,BaseEntityComponentHandle>,CFogControllerHandle>(l,defCFogController);
	entsMod[defCFogController];

	auto defCLight = luabind::class_<CLightHandle,BaseEntityComponentHandle>("LightComponent");
	Lua::register_base_env_light_component_methods<luabind::class_<CLightHandle,BaseEntityComponentHandle>,CLightHandle>(l,defCLight);
	defCLight.def("SetShadowType",static_cast<void(*)(lua_State*,CLightHandle&,uint32_t)>([](lua_State *l,CLightHandle &hComponent,uint32_t type) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetShadowType(static_cast<pragma::BaseEnvLightComponent::ShadowType>(type));
	}));
	defCLight.def("GetShadowType",static_cast<void(*)(lua_State*,CLightHandle&)>([](lua_State *l,CLightHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,umath::to_integral(hComponent->GetShadowType()));
	}));
	defCLight.def("UpdateBuffers",static_cast<void(*)(lua_State*,CLightHandle&)>([](lua_State *l,CLightHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->UpdateBuffers();
	}));
	defCLight.def("SetAddToGameScene",static_cast<void(*)(lua_State*,CLightHandle&,bool)>([](lua_State *l,CLightHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetStateFlag(pragma::CLightComponent::StateFlags::AddToGameScene,b);
	}));
	defCLight.add_static_constant("SHADOW_TYPE_NONE",umath::to_integral(ShadowType::None));
	defCLight.add_static_constant("SHADOW_TYPE_STATIC_ONLY",umath::to_integral(ShadowType::StaticOnly));
	defCLight.add_static_constant("SHADOW_TYPE_FULL",umath::to_integral(ShadowType::Full));

	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY",pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY_MESH",pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_MESH",pragma::CLightComponent::EVENT_SHOULD_PASS_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_UPDATE_RENDER_PASS",pragma::CLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS);
	defCLight.add_static_constant("EVENT_GET_TRANSFORMATION_MATRIX",pragma::CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX);
	defCLight.add_static_constant("EVENT_HANDLE_SHADOW_MAP",pragma::CLightComponent::EVENT_HANDLE_SHADOW_MAP);
	defCLight.add_static_constant("EVENT_ON_SHADOW_BUFFER_INITIALIZED",pragma::CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED);
	entsMod[defCLight];

	auto defCLightDirectional = luabind::class_<CLightDirectionalHandle,BaseEntityComponentHandle>("LightDirectionalComponent");
	Lua::register_base_env_light_directional_component_methods<luabind::class_<CLightDirectionalHandle,BaseEntityComponentHandle>,CLightDirectionalHandle>(l,defCLightDirectional);
	entsMod[defCLightDirectional];

	auto defCLightPoint = luabind::class_<CLightPointHandle,BaseEntityComponentHandle>("LightPointComponent");
	Lua::register_base_env_light_point_component_methods<luabind::class_<CLightPointHandle,BaseEntityComponentHandle>,CLightPointHandle>(l,defCLightPoint);
	entsMod[defCLightPoint];

	auto defCLightSpot = luabind::class_<CLightSpotHandle,BaseEntityComponentHandle>("LightSpotComponent");
	Lua::register_base_env_light_spot_component_methods<luabind::class_<CLightSpotHandle,BaseEntityComponentHandle>,CLightSpotHandle>(l,defCLightSpot);
	entsMod[defCLightSpot];

	auto defCLightSpotVol = luabind::class_<CLightSpotVolHandle,BaseEntityComponentHandle>("LightSpotVolComponent");
	Lua::register_base_env_light_spot_vol_component_methods<luabind::class_<CLightSpotVolHandle,BaseEntityComponentHandle>,CLightSpotVolHandle>(l,defCLightSpotVol);
	entsMod[defCLightSpotVol];

	auto defCMicrophone = luabind::class_<CMicrophoneHandle,BaseEntityComponentHandle>("MicrophoneComponent");
	Lua::register_base_env_microphone_component_methods<luabind::class_<CMicrophoneHandle,BaseEntityComponentHandle>,CMicrophoneHandle>(l,defCMicrophone);
	entsMod[defCMicrophone];

	Lua::ParticleSystem::register_class(l,entsMod);

	auto defCQuake = luabind::class_<CQuakeHandle,BaseEntityComponentHandle>("QuakeComponent");
	Lua::register_base_env_quake_component_methods<luabind::class_<CQuakeHandle,BaseEntityComponentHandle>,CQuakeHandle>(l,defCQuake);
	entsMod[defCQuake];

	auto defCSmokeTrail = luabind::class_<CSmokeTrailHandle,BaseEntityComponentHandle>("SmokeTrailComponent");
	Lua::register_base_env_smoke_trail_component_methods<luabind::class_<CSmokeTrailHandle,BaseEntityComponentHandle>,CSmokeTrailHandle>(l,defCSmokeTrail);
	entsMod[defCSmokeTrail];

	auto defCSound = luabind::class_<CSoundHandle,BaseEntityComponentHandle>("SoundComponent");
	Lua::register_base_env_sound_component_methods<luabind::class_<CSoundHandle,BaseEntityComponentHandle>,CSoundHandle>(l,defCSound);
	entsMod[defCSound];

	auto defCSoundScape = luabind::class_<CSoundScapeHandle,BaseEntityComponentHandle>("SoundScapeComponent");
	Lua::register_base_env_soundscape_component_methods<luabind::class_<CSoundScapeHandle,BaseEntityComponentHandle>,CSoundScapeHandle>(l,defCSoundScape);
	entsMod[defCSoundScape];

	auto defCSprite = luabind::class_<CSpriteHandle,BaseEntityComponentHandle>("SpriteComponent");
	Lua::register_base_env_sprite_component_methods<luabind::class_<CSpriteHandle,BaseEntityComponentHandle>,CSpriteHandle>(l,defCSprite);
	defCSprite.def("StopAndRemoveEntity",static_cast<void(*)(lua_State*,CSpriteHandle&)>([](lua_State *l,CSpriteHandle &hSprite) {
		pragma::Lua::check_component(l,hSprite);
		hSprite->StopAndRemoveEntity();
	}));
	entsMod[defCSprite];

	auto defCTimescale = luabind::class_<CEnvTimescaleHandle,BaseEntityComponentHandle>("EnvTimescaleComponent");
	Lua::register_base_env_timescale_component_methods<luabind::class_<CEnvTimescaleHandle,BaseEntityComponentHandle>,CEnvTimescaleHandle>(l,defCTimescale);
	entsMod[defCTimescale];

	auto defCWind = luabind::class_<CWindHandle,BaseEntityComponentHandle>("WindComponent");
	Lua::register_base_env_wind_component_methods<luabind::class_<CWindHandle,BaseEntityComponentHandle>,CWindHandle>(l,defCWind);
	entsMod[defCWind];

	auto defCFilterClass = luabind::class_<CFilterClassHandle,BaseEntityComponentHandle>("FilterClassComponent");
	Lua::register_base_env_filter_class_component_methods<luabind::class_<CFilterClassHandle,BaseEntityComponentHandle>,CFilterClassHandle>(l,defCFilterClass);
	entsMod[defCFilterClass];

	auto defCFilterName = luabind::class_<CFilterNameHandle,BaseEntityComponentHandle>("FilterNameComponent");
	Lua::register_base_env_filter_name_component_methods<luabind::class_<CFilterNameHandle,BaseEntityComponentHandle>,CFilterNameHandle>(l,defCFilterName);
	entsMod[defCFilterName];

	auto defCBrush = luabind::class_<CBrushHandle,BaseEntityComponentHandle>("BrushComponent");
	Lua::register_base_func_brush_component_methods<luabind::class_<CBrushHandle,BaseEntityComponentHandle>,CBrushHandle>(l,defCBrush);
	entsMod[defCBrush];

	auto defCKinematic = luabind::class_<CKinematicHandle,BaseEntityComponentHandle>("KinematicComponent");
	Lua::register_base_func_kinematic_component_methods<luabind::class_<CKinematicHandle,BaseEntityComponentHandle>,CKinematicHandle>(l,defCKinematic);
	entsMod[defCKinematic];

	auto defCFuncPhysics = luabind::class_<CFuncPhysicsHandle,BaseEntityComponentHandle>("FuncPhysicsComponent");
	Lua::register_base_func_physics_component_methods<luabind::class_<CFuncPhysicsHandle,BaseEntityComponentHandle>,CFuncPhysicsHandle>(l,defCFuncPhysics);
	entsMod[defCFuncPhysics];

	auto defCFuncSoftPhysics = luabind::class_<CFuncSoftPhysicsHandle,BaseEntityComponentHandle>("FuncSoftPhysicsComponent");
	Lua::register_base_func_soft_physics_component_methods<luabind::class_<CFuncSoftPhysicsHandle,BaseEntityComponentHandle>,CFuncSoftPhysicsHandle>(l,defCFuncSoftPhysics);
	entsMod[defCFuncSoftPhysics];

	auto defCFuncPortal = luabind::class_<CFuncPortalHandle,BaseEntityComponentHandle>("FuncPortalComponent");
	Lua::register_base_func_portal_component_methods<luabind::class_<CFuncPortalHandle,BaseEntityComponentHandle>,CFuncPortalHandle>(l,defCFuncPortal);
	entsMod[defCFuncPortal];

	auto defCWater = luabind::class_<CWaterHandle,BaseEntityComponentHandle>("WaterComponent");
	Lua::register_base_func_water_component_methods<luabind::class_<CWaterHandle,BaseEntityComponentHandle>,CWaterHandle>(l,defCWater);
	defCWater.def("GetReflectionScene",static_cast<void(*)(lua_State*,CWaterHandle&)>([](lua_State *l,CWaterHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		if(hEnt->IsWaterSceneValid() == false)
			return;
		hEnt->GetWaterScene().sceneReflection->GetLuaObject().push(l);
	}));
	defCWater.def("GetWaterSceneTexture",static_cast<void(*)(lua_State*,CWaterHandle&)>([](lua_State *l,CWaterHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		if(hEnt->IsWaterSceneValid() == false)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,hEnt->GetWaterScene().texScene);
	}));
	defCWater.def("GetWaterSceneDepthTexture",static_cast<void(*)(lua_State*,CWaterHandle&)>([](lua_State *l,CWaterHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		if(hEnt->IsWaterSceneValid() == false)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,hEnt->GetWaterScene().texSceneDepth);
	}));
	entsMod[defCWater];

	auto defCButton = luabind::class_<CButtonHandle,BaseEntityComponentHandle>("ButtonComponent");
	Lua::register_base_func_button_component_methods<luabind::class_<CButtonHandle,BaseEntityComponentHandle>,CButtonHandle>(l,defCButton);
	entsMod[defCButton];

	auto defCBot = luabind::class_<CBotHandle,BaseEntityComponentHandle>("BotComponent");
	Lua::register_base_bot_component_methods<luabind::class_<CBotHandle,BaseEntityComponentHandle>,CBotHandle>(l,defCBot);
	entsMod[defCBot];

	auto defCPointConstraintBallSocket = luabind::class_<CPointConstraintBallSocketHandle,BaseEntityComponentHandle>("PointConstraintBallSocketComponent");
	Lua::register_base_point_constraint_ball_socket_component_methods<luabind::class_<CPointConstraintBallSocketHandle,BaseEntityComponentHandle>,CPointConstraintBallSocketHandle>(l,defCPointConstraintBallSocket);
	entsMod[defCPointConstraintBallSocket];

	auto defCPointConstraintConeTwist = luabind::class_<CPointConstraintConeTwistHandle,BaseEntityComponentHandle>("PointConstraintConeTwistComponent");
	Lua::register_base_point_constraint_cone_twist_component_methods<luabind::class_<CPointConstraintConeTwistHandle,BaseEntityComponentHandle>,CPointConstraintConeTwistHandle>(l,defCPointConstraintConeTwist);
	entsMod[defCPointConstraintConeTwist];

	auto defCPointConstraintDoF = luabind::class_<CPointConstraintDoFHandle,BaseEntityComponentHandle>("PointConstraintDoFComponent");
	Lua::register_base_point_constraint_dof_component_methods<luabind::class_<CPointConstraintDoFHandle,BaseEntityComponentHandle>,CPointConstraintDoFHandle>(l,defCPointConstraintDoF);
	entsMod[defCPointConstraintDoF];

	auto defCPointConstraintFixed = luabind::class_<CPointConstraintFixedHandle,BaseEntityComponentHandle>("PointConstraintFixedComponent");
	Lua::register_base_point_constraint_fixed_component_methods<luabind::class_<CPointConstraintFixedHandle,BaseEntityComponentHandle>,CPointConstraintFixedHandle>(l,defCPointConstraintFixed);
	entsMod[defCPointConstraintFixed];

	auto defCPointConstraintHinge = luabind::class_<CPointConstraintHingeHandle,BaseEntityComponentHandle>("PointConstraintHingeComponent");
	Lua::register_base_point_constraint_hinge_component_methods<luabind::class_<CPointConstraintHingeHandle,BaseEntityComponentHandle>,CPointConstraintHingeHandle>(l,defCPointConstraintHinge);
	entsMod[defCPointConstraintHinge];

	auto defCPointConstraintSlider = luabind::class_<CPointConstraintSliderHandle,BaseEntityComponentHandle>("PointConstraintSliderComponent");
	Lua::register_base_point_constraint_slider_component_methods<luabind::class_<CPointConstraintSliderHandle,BaseEntityComponentHandle>,CPointConstraintSliderHandle>(l,defCPointConstraintSlider);
	entsMod[defCPointConstraintSlider];

	auto defCRenderTarget = luabind::class_<CRenderTargetHandle,BaseEntityComponentHandle>("RenderTargetComponent");
	Lua::register_base_point_render_target_component_methods<luabind::class_<CRenderTargetHandle,BaseEntityComponentHandle>,CRenderTargetHandle>(l,defCRenderTarget);
	entsMod[defCRenderTarget];

	auto defCPointTarget = luabind::class_<CPointTargetHandle,BaseEntityComponentHandle>("PointTargetComponent");
	Lua::register_base_point_target_component_methods<luabind::class_<CPointTargetHandle,BaseEntityComponentHandle>,CPointTargetHandle>(l,defCPointTarget);
	entsMod[defCPointTarget];

	auto defCProp = luabind::class_<CPropHandle,BaseEntityComponentHandle>("PropComponent");
	Lua::register_base_prop_component_methods<luabind::class_<CPropHandle,BaseEntityComponentHandle>,CPropHandle>(l,defCProp);
	entsMod[defCProp];

	auto defCPropDynamic = luabind::class_<CPropDynamicHandle,BaseEntityComponentHandle>("PropDynamicComponent");
	Lua::register_base_prop_dynamic_component_methods<luabind::class_<CPropDynamicHandle,BaseEntityComponentHandle>,CPropDynamicHandle>(l,defCPropDynamic);
	entsMod[defCPropDynamic];

	auto defCPropPhysics = luabind::class_<CPropPhysicsHandle,BaseEntityComponentHandle>("PropPhysicsComponent");
	Lua::register_base_prop_physics_component_methods<luabind::class_<CPropPhysicsHandle,BaseEntityComponentHandle>,CPropPhysicsHandle>(l,defCPropPhysics);
	entsMod[defCPropPhysics];

	auto defCTouch = luabind::class_<CTouchHandle,BaseEntityComponentHandle>("TouchComponent");
	Lua::register_base_touch_component_methods<luabind::class_<CTouchHandle,BaseEntityComponentHandle>,CTouchHandle>(l,defCTouch);
	entsMod[defCTouch];

	auto defCSkybox = luabind::class_<CSkyboxHandle,BaseEntityComponentHandle>("SkyboxComponent");
	Lua::register_base_skybox_component_methods<luabind::class_<CSkyboxHandle,BaseEntityComponentHandle>,CSkyboxHandle>(l,defCSkybox);
	entsMod[defCSkybox];

	auto defCFlashlight = luabind::class_<CFlashlightHandle,BaseEntityComponentHandle>("FlashlightComponent");
	Lua::register_base_flashlight_component_methods<luabind::class_<CFlashlightHandle,BaseEntityComponentHandle>,CFlashlightHandle>(l,defCFlashlight);
	entsMod[defCFlashlight];

	auto defCEnvSoundProbe = luabind::class_<CEnvSoundProbeHandle,BaseEntityComponentHandle>("EnvSoundProbeComponent");
	entsMod[defCEnvSoundProbe];

	auto defCWeather = luabind::class_<CWeatherHandle,BaseEntityComponentHandle>("WeatherComponent");
	Lua::register_base_env_weather_component_methods<luabind::class_<CWeatherHandle,BaseEntityComponentHandle>,CWeatherHandle>(l,defCWeather);
	entsMod[defCWeather];

	auto defCReflectionProbe = luabind::class_<CReflectionProbeHandle,BaseEntityComponentHandle>("ReflectionProbeComponent");
	defCReflectionProbe.def("GetIBLStrength",static_cast<void(*)(lua_State*,CReflectionProbeHandle&)>([](lua_State *l,CReflectionProbeHandle &hRp) {
		pragma::Lua::check_component(l,hRp);
		Lua::PushNumber(l,hRp->GetIBLStrength());
	}));
	defCReflectionProbe.def("SetIBLStrength",static_cast<void(*)(lua_State*,CReflectionProbeHandle&,float)>([](lua_State *l,CReflectionProbeHandle &hRp,float strength) {
		pragma::Lua::check_component(l,hRp);
		hRp->SetIBLStrength(strength);
	}));
	entsMod[defCReflectionProbe];

	auto defCSkyCamera = luabind::class_<CSkyCameraHandle,BaseEntityComponentHandle>("SkyCameraComponent");
	entsMod[defCSkyCamera];

	auto defCPBRConverter = luabind::class_<CPBRConverterHandle,BaseEntityComponentHandle>("PBRConverterComponent");
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,Model&,uint32_t,uint32_t,uint32_t,bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,Model&,uint32_t,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,Model&,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,Model&)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,EntityHandle&,uint32_t,uint32_t,uint32_t,bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,EntityHandle&,uint32_t,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,EntityHandle&,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,EntityHandle&)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	entsMod[defCPBRConverter];

	auto defShadow = luabind::class_<CShadowHandle,BaseEntityComponentHandle>("ShadowMapComponent");
	entsMod[defShadow];

	auto defShadowCsm = luabind::class_<CShadowCSMHandle,BaseEntityComponentHandle>("CSMComponent");
	entsMod[defShadowCsm];

	auto defShadowManager = luabind::class_<CShadowManagerHandle,BaseEntityComponentHandle>("ShadowManagerComponent");
	entsMod[defShadowManager];

	auto defCWaterSurface = luabind::class_<CWaterSurfaceHandle,BaseEntityComponentHandle>("WaterSurfaceComponent");
	entsMod[defCWaterSurface];

	auto defCListener = luabind::class_<CListenerHandle,BaseEntityComponentHandle>("ListenerComponent");
	entsMod[defCListener];

	auto defCViewBody = luabind::class_<CViewBodyHandle,BaseEntityComponentHandle>("ViewBodyComponent");
	entsMod[defCViewBody];

	auto defCViewModel = luabind::class_<CViewModelHandle,BaseEntityComponentHandle>("ViewModelComponent");
	entsMod[defCViewModel];

	auto defCSoftBody = luabind::class_<CSoftBodyHandle,BaseEntityComponentHandle>("SoftBodyComponent");
	entsMod[defCSoftBody];

	auto defCRaytracing = luabind::class_<CRaytracingHandle,BaseEntityComponentHandle>("RaytracingComponent");
	entsMod[defCRaytracing];

	auto defCBSPLeaf = luabind::class_<CBSPLeafHandle,BaseEntityComponentHandle>("BSPLeafComponent");
	entsMod[defCBSPLeaf];

	auto defCIo = luabind::class_<CIOHandle,BaseEntityComponentHandle>("IOComponent");
	Lua::register_base_io_component_methods<luabind::class_<CIOHandle,BaseEntityComponentHandle>,CIOHandle>(l,defCIo);
	entsMod[defCIo];

	auto defCTimeScale = luabind::class_<CTimeScaleHandle,BaseEntityComponentHandle>("TimeScaleComponent");
	Lua::register_base_time_scale_component_methods<luabind::class_<CTimeScaleHandle,BaseEntityComponentHandle>,CTimeScaleHandle>(l,defCTimeScale);
	entsMod[defCTimeScale];

	auto defCAttachable = luabind::class_<CAttachableHandle,BaseEntityComponentHandle>("AttachableComponent");
	Lua::register_base_attachable_component_methods<luabind::class_<CAttachableHandle,BaseEntityComponentHandle>,CAttachableHandle>(l,defCAttachable);
	entsMod[defCAttachable];

	auto defCParent = luabind::class_<CParentHandle,BaseEntityComponentHandle>("ParentComponent");
	Lua::register_base_parent_component_methods<luabind::class_<CParentHandle,BaseEntityComponentHandle>,CParentHandle>(l,defCParent);
	entsMod[defCParent];
	
	auto defCOwnable = luabind::class_<COwnableHandle,BaseEntityComponentHandle>("OwnableComponent");
	Lua::register_base_ownable_component_methods<luabind::class_<COwnableHandle,BaseEntityComponentHandle>,COwnableHandle>(l,defCOwnable);
	entsMod[defCOwnable];

	auto defCDebugText = luabind::class_<CDebugTextHandle,BaseEntityComponentHandle>("DebugTextComponent");
	Lua::register_base_debug_text_component_methods<luabind::class_<CDebugTextHandle,BaseEntityComponentHandle>,CDebugTextHandle>(l,defCDebugText);
	entsMod[defCDebugText];

	auto defCDebugPoint = luabind::class_<CDebugPointHandle,BaseEntityComponentHandle>("DebugPointComponent");
	Lua::register_base_debug_point_component_methods<luabind::class_<CDebugPointHandle,BaseEntityComponentHandle>,CDebugPointHandle>(l,defCDebugPoint);
	entsMod[defCDebugPoint];

	auto defCDebugLine = luabind::class_<CDebugLineHandle,BaseEntityComponentHandle>("DebugLineComponent");
	Lua::register_base_debug_line_component_methods<luabind::class_<CDebugLineHandle,BaseEntityComponentHandle>,CDebugLineHandle>(l,defCDebugLine);
	entsMod[defCDebugLine];

	auto defCDebugBox = luabind::class_<CDebugBoxHandle,BaseEntityComponentHandle>("DebugBoxComponent");
	Lua::register_base_debug_box_component_methods<luabind::class_<CDebugBoxHandle,BaseEntityComponentHandle>,CDebugBoxHandle>(l,defCDebugBox);
	entsMod[defCDebugBox];

	auto defCDebugSphere = luabind::class_<CDebugSphereHandle,BaseEntityComponentHandle>("DebugSphereComponent");
	Lua::register_base_debug_sphere_component_methods<luabind::class_<CDebugSphereHandle,BaseEntityComponentHandle>,CDebugSphereHandle>(l,defCDebugSphere);
	entsMod[defCDebugSphere];

	auto defCDebugCone = luabind::class_<CDebugConeHandle,BaseEntityComponentHandle>("DebugConeComponent");
	Lua::register_base_debug_cone_component_methods<luabind::class_<CDebugConeHandle,BaseEntityComponentHandle>,CDebugConeHandle>(l,defCDebugCone);
	entsMod[defCDebugCone];

	auto defCDebugCylinder = luabind::class_<CDebugCylinderHandle,BaseEntityComponentHandle>("DebugCylinderComponent");
	Lua::register_base_debug_cylinder_component_methods<luabind::class_<CDebugCylinderHandle,BaseEntityComponentHandle>,CDebugCylinderHandle>(l,defCDebugCylinder);
	entsMod[defCDebugCylinder];

	auto defCDebugPlane = luabind::class_<CDebugPlaneHandle,BaseEntityComponentHandle>("DebugPlaneComponent");
	Lua::register_base_debug_plane_component_methods<luabind::class_<CDebugPlaneHandle,BaseEntityComponentHandle>,CDebugPlaneHandle>(l,defCDebugPlane);
	entsMod[defCDebugPlane];

	auto defCPointAtTarget = luabind::class_<CPointAtTargetHandle,BaseEntityComponentHandle>("PointAtTargetComponent");
	Lua::register_base_point_at_target_component_methods<luabind::class_<CPointAtTargetHandle,BaseEntityComponentHandle>,CPointAtTargetHandle>(l,defCPointAtTarget);
	entsMod[defCPointAtTarget];

	auto defCBSP = luabind::class_<CBSPHandle,BaseEntityComponentHandle>("BSPComponent");
	entsMod[defCBSP];

	auto defCGeneric = luabind::class_<CGenericHandle,BaseEntityComponentHandle>("EntityComponent");
	//Lua::register_base_generic_component_methods<luabind::class_<CGenericHandle,BaseEntityComponentHandle>,CGenericHandle>(l,defCGeneric);
	entsMod[defCGeneric];
}

//////////////

void Lua::Flex::GetFlexController(lua_State *l,CFlexHandle &hEnt,uint32_t flexId)
{
	pragma::Lua::check_component(l,hEnt);
	auto val = 0.f;
	if(hEnt->GetFlexController(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}
void Lua::Flex::GetFlexController(lua_State *l,CFlexHandle &hEnt,const std::string &flexController)
{
	pragma::Lua::check_component(l,hEnt);
	auto flexId = 0u;
	auto mdlComponent = hEnt->GetEntity().GetModelComponent();
	if(mdlComponent.expired() || mdlComponent->LookupFlexController(flexController,flexId) == false)
		return;
	auto val = 0.f;
	if(hEnt->GetFlexController(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}
void Lua::Flex::CalcFlexValue(lua_State *l,CFlexHandle &hEnt,uint32_t flexId)
{
	pragma::Lua::check_component(l,hEnt);
	auto val = 0.f;
	if(hEnt->CalcFlexValue(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}

//////////////

void Lua::SoundEmitter::CreateSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->CreateSound(sndname,static_cast<ALSoundType>(soundType));
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
void Lua::SoundEmitter::EmitSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}

//////////////

void Lua::ParticleSystem::Stop(lua_State *l,CParticleSystemHandle &hComponent,bool bStopImmediately)
{
	pragma::Lua::check_component(l,hComponent);
	if(bStopImmediately == true)
		hComponent->Stop();
	else
		hComponent->Die();
}
void Lua::ParticleSystem::AddInitializer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);
	
	auto ot = luabind::object{luabind::from_stack{l,t}};
	for(luabind::iterator i{ot},end;i!=end;++i)
	{
		auto key = luabind::object_cast<std::string>(i.key());
		auto &valRef = *i;
		valRef.push(l);
		auto idx = Lua::GetStackTop(l);
		std::string val = Lua::ParticleSystem::get_key_value(l,idx);
		Lua::Pop(l,1);

		ustring::to_lower(key);
		values[key] = val;
	}

	auto *initializer = hComponent.AddInitializer(name,values);
	if(initializer == nullptr)
		return;
	Lua::Push(l,initializer);
}
void Lua::ParticleSystem::AddOperator(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);

	auto ot = luabind::object{luabind::from_stack{l,t}};
	for(luabind::iterator i{ot},end;i!=end;++i)
	{
		auto key = luabind::object_cast<std::string>(i.key());
		auto &valRef = *i;
		valRef.push(l);
		auto idx = Lua::GetStackTop(l);
		std::string val = Lua::ParticleSystem::get_key_value(l,idx);
		Lua::Pop(l,1);

		ustring::to_lower(key);
		values[key] = val;
	}

	auto *op = hComponent.AddOperator(name,values);
	if(op == nullptr)
		return;
	Lua::Push(l,op);
}
void Lua::ParticleSystem::AddRenderer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);

	auto ot = luabind::object{luabind::from_stack{l,t}};
	for(luabind::iterator i{ot},end;i!=end;++i)
	{
		auto key = luabind::object_cast<std::string>(i.key());
		auto &valRef = *i;
		valRef.push(l);
		auto idx = Lua::GetStackTop(l);
		std::string val = Lua::ParticleSystem::get_key_value(l,idx);
		Lua::Pop(l,1);

		ustring::to_lower(key);
		values[key] = val;
	}

	auto *renderer = hComponent.AddRenderer(name,values);
	if(renderer == nullptr)
		return;
	Lua::Push(l,renderer);
}
