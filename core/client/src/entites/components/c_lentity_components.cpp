#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/entities/components/c_lentity_components.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/model/c_modelmesh.h"
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
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>

namespace Lua
{
	namespace Render
	{
		static void GetModelMatrix(lua_State *l,CRenderHandle &hEnt);
		static void GetTranslationMatrix(lua_State *l,CRenderHandle &hEnt);
		static void GetRotationMatrix(lua_State *l,CRenderHandle &hEnt);
		static void GetTransformationMatrix(lua_State *l,CRenderHandle &hEnt);
		static void RenderModel(lua_State *l,CRenderHandle &hEnt);
		static void SetRenderMode(lua_State *l,CRenderHandle &hEnt,unsigned int mode);
		static void GetRenderMode(lua_State *l,CRenderHandle &hEnt);
		static void GetRenderBounds(lua_State *l,CRenderHandle &hEnt);
		static void GetRenderSphereBounds(lua_State *l,CRenderHandle &hEnt);
		static void SetRenderBounds(lua_State *l,CRenderHandle &hEnt,Vector3 &min,Vector3 &max);
		static void UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::CommandBuffer> &drawCmd,bool bForceBufferUpdate);
		static void UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::CommandBuffer> &drawCmd);
		static void GetRenderBuffer(lua_State *l,CRenderHandle &hEnt);
		static void GetBoneBuffer(lua_State *l,CRenderHandle &hEnt);
	};
	namespace SoundEmitter
	{
		static void CreateSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t type,bool bTransmit);
		static void EmitSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit);
	};
	namespace Flex
	{
		static void GetFlexController(lua_State *l,CFlexHandle &hEnt,uint32_t flexId);
		static void GetFlexController(lua_State *l,CFlexHandle &hEnt,const std::string &flexController);
		static void SetFlexController(lua_State *l,CFlexHandle &hEnt,uint32_t flexId,float cycle);
		static void SetFlexController(lua_State *l,CFlexHandle &hEnt,const std::string &flexController,float cycle);
		static void CalcFlexValue(lua_State *l,CFlexHandle &hEnt,uint32_t flexId);
	};
	namespace ParticleSystem
	{
		static void Stop(lua_State *l,CParticleSystemHandle &hComponent,bool bStopImmediately);
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

	auto defCRender = luabind::class_<CRenderHandle,BaseEntityComponentHandle>("RenderComponent");
	Lua::register_base_render_component_methods<luabind::class_<CRenderHandle,BaseEntityComponentHandle>,CRenderHandle>(l,defCRender);
	defCRender.def("GetModelMatrix",&Lua::Render::GetModelMatrix);
	defCRender.def("GetTranslationMatrix",&Lua::Render::GetTranslationMatrix);
	defCRender.def("GetRotationMatrix",&Lua::Render::GetRotationMatrix);
	defCRender.def("GetTransformationMatrix",&Lua::Render::GetTransformationMatrix);
	defCRender.def("SetRenderMode",&Lua::Render::SetRenderMode);
	defCRender.def("GetRenderMode",&Lua::Render::GetRenderMode);
	defCRender.def("GetRenderModeProperty",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetRenderModeProperty());
	}));
	defCRender.def("GetRenderBounds",&Lua::Render::GetRenderBounds);
	defCRender.def("SetRenderBounds",&Lua::Render::SetRenderBounds);
	defCRender.def("GetRenderSphereBounds",&Lua::Render::GetRenderSphereBounds);
	defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua_State*,CRenderHandle&,std::shared_ptr<prosper::CommandBuffer>&,bool)>(&Lua::Render::UpdateRenderBuffers));
	defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua_State*,CRenderHandle&,std::shared_ptr<prosper::CommandBuffer>&)>(&Lua::Render::UpdateRenderBuffers));
	defCRender.def("GetRenderBuffer",&Lua::Render::GetRenderBuffer);
	defCRender.def("GetBoneBuffer",&Lua::Render::GetBoneBuffer);

	auto &componentManager = engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager();
	defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_DATA",pragma::CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA);
	defCRender.add_static_constant("EVENT_ON_RENDER_BOUNDS_CHANGED",pragma::CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED);
	defCRender.add_static_constant("EVENT_ON_RENDER_BUFFERS_INITIALIZED",pragma::CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW",pragma::CRenderComponent::EVENT_SHOULD_DRAW);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW_SHADOW",pragma::CRenderComponent::EVENT_SHOULD_DRAW_SHADOW);
	defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_MATRICES",pragma::CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES);

	// Enums
	defCRender.add_static_constant("RENDERMODE_NONE",umath::to_integral(RenderMode::None));
	defCRender.add_static_constant("RENDERMODE_AUTO",umath::to_integral(RenderMode::Auto));
	defCRender.add_static_constant("RENDERMODE_WORLD",umath::to_integral(RenderMode::World));
	defCRender.add_static_constant("RENDERMODE_VIEW",umath::to_integral(RenderMode::View));
	defCRender.add_static_constant("RENDERMODE_SKYBOX",umath::to_integral(RenderMode::Skybox));
	defCRender.add_static_constant("RENDERMODE_WATER",umath::to_integral(RenderMode::Water));
	entsMod[defCRender];

	auto defCSoundEmitter = luabind::class_<CSoundEmitterHandle,BaseEntityComponentHandle>("SoundEmitterComponent");
	Lua::register_base_sound_emitter_component_methods<luabind::class_<CSoundEmitterHandle,BaseEntityComponentHandle>,CSoundEmitterHandle>(l,defCSoundEmitter);
	defCSoundEmitter.def("CreateSound",static_cast<void(*)(lua_State*,CSoundEmitterHandle&,std::string,uint32_t,bool)>(&Lua::SoundEmitter::CreateSound));
	defCSoundEmitter.def("EmitSound",static_cast<void(*)(lua_State*,CSoundEmitterHandle&,std::string,uint32_t,float,float,bool)>(&Lua::SoundEmitter::EmitSound));
	entsMod[defCSoundEmitter];

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

	auto defCParticleSystem = luabind::class_<CParticleSystemHandle,BaseEntityComponentHandle>("ParticleSystemComponent");
	Lua::register_base_env_particle_system_component_methods<luabind::class_<CParticleSystemHandle,BaseEntityComponentHandle>,CParticleSystemHandle>(l,defCParticleSystem);
	defCParticleSystem.def("Start",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Start();
	}));
	defCParticleSystem.def("Stop",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		Lua::ParticleSystem::Stop(l,hComponent,false);
	}));
	defCParticleSystem.def("Stop",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool bStopImmediately) {
		Lua::ParticleSystem::Stop(l,hComponent,bStopImmediately);
	}));
	defCParticleSystem.def("AddInitializer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::string,luabind::object)>([](lua_State *l,CParticleSystemHandle &hComponent,std::string name,luabind::object o) {
		pragma::Lua::check_component(l,hComponent);
		Lua::ParticleSystem::AddInitializer(l,*hComponent,name,o);
	}));
	defCParticleSystem.def("AddOperator",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::string,luabind::object)>([](lua_State *l,CParticleSystemHandle &hComponent,std::string name,luabind::object o) {
		pragma::Lua::check_component(l,hComponent);
		Lua::ParticleSystem::AddOperator(l,*hComponent,name,o);
	}));
	defCParticleSystem.def("AddRenderer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::string,luabind::object)>([](lua_State *l,CParticleSystemHandle &hComponent,std::string name,luabind::object o) {
		pragma::Lua::check_component(l,hComponent);
		Lua::ParticleSystem::AddRenderer(l,*hComponent,name,o);
	}));
	defCParticleSystem.def("AddChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto *pt = hComponent.get()->AddChild(name);
		if(pt == nullptr)
			return;
		Lua::Push(l,pt->GetHandle());
	}));
	defCParticleSystem.def("AddChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hChild) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hChild);
		hComponent.get()->AddChild(*hChild);
	}));
	defCParticleSystem.def("SetNodeTarget",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,EntityHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId,EntityHandle &hEnt) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetNodeTarget(nodeId,static_cast<CBaseEntity*>(hEnt.get()));
	}));
	defCParticleSystem.def("SetNodeTarget",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,const Vector3&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId,const Vector3 &pos) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetNodeTarget(nodeId,pos);
	}));
	defCParticleSystem.def("GetNodeCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetNodeCount());
	}));
	defCParticleSystem.def("GetNodePosition",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Push<Vector3>(l,hComponent->GetNodePosition(nodeId));
	}));
	defCParticleSystem.def("GetNodeTarget",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId) {
		pragma::Lua::check_component(l,hComponent);
		auto *ent = hComponent->GetNodeTarget(nodeId);
		if(ent == nullptr)
			return;
		ent->GetLuaObject()->push(l);
	}));
	defCParticleSystem.def("SetRemoveOnComplete",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRemoveOnComplete(b);
	}));
	defCParticleSystem.def("GetRenderBounds",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &bounds = hComponent->GetRenderBounds();
		Lua::Push<Vector3>(l,bounds.first);
		Lua::Push<Vector3>(l,bounds.second);
	}));
	defCParticleSystem.def("SetRadius",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float radius) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRadius(radius);
	}));
	defCParticleSystem.def("GetRadius",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetRadius());
	}));
	defCParticleSystem.def("SetExtent",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float extent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetExtent(extent);
	}));
	defCParticleSystem.def("GetExtent",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetExtent());
	}));
	defCParticleSystem.def("SetMaterial",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetMaterial(name.c_str());
	}));
	defCParticleSystem.def("SetMaterial",static_cast<void(*)(lua_State*,CParticleSystemHandle&,Material*)>([](lua_State *l,CParticleSystemHandle &hComponent,Material *mat) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetMaterial(mat);
	}));
	defCParticleSystem.def("GetMaterial",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *mat = hComponent->GetMaterial();
		if(mat == nullptr)
			return;
		Lua::Push<Material*>(l,mat);
	}));
	defCParticleSystem.def("SetOrientationType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t orientationType) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetOrientationType(static_cast<pragma::CParticleSystemComponent::OrientationType>(orientationType));
	}));
	defCParticleSystem.def("GetOrientationType",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,umath::to_integral(hComponent->GetOrientationType()));
	}));
	defCParticleSystem.def("IsContinuous",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsContinuous());
	}));
	defCParticleSystem.def("SetContinuous",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetContinuous(b);
	}));
	defCParticleSystem.def("GetRemoveOnComplete",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetRemoveOnComplete());
	}));
	defCParticleSystem.def("GetCastShadows",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetCastShadows());
	}));
	defCParticleSystem.def("SetCastShadows",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetCastShadows(b);
	}));
	defCParticleSystem.def("GetBloomScale",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetBloomScale());
	}));
	defCParticleSystem.def("SetBloomScale",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float scale) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetBloomScale(scale);
	}));
	defCParticleSystem.def("GetIntensity",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetIntensity());
	}));
	defCParticleSystem.def("SetIntensity",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float intensity) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetIntensity(intensity);
	}));
	defCParticleSystem.def("GetParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetParticleCount());
	}));
	defCParticleSystem.def("GetMaxParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetMaxParticleCount());
	}));
	defCParticleSystem.def("Die",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Die();
	}));
	defCParticleSystem.def("IsActive",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsActive());
	}));
	defCParticleSystem.def("GetRenderMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetRenderMode());
	}));
	defCParticleSystem.def("SetRenderMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t renderMode) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRenderMode(static_cast<RenderMode>(renderMode));
	}));
	defCParticleSystem.def("SetName",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetParticleSystemName(name);
	}));
	defCParticleSystem.def("GetName",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushString(l,hComponent->GetParticleSystemName());
	}));
	defCParticleSystem.def("IsStatic",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsStatic());
	}));
	defCParticleSystem.def("GetChildren",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &children = hComponent->GetChildren();
		auto t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto &hChild : children)
		{
			if(hChild.expired())
				continue;
			Lua::PushInt(l,idx++);
			Lua::Push(l,hChild);
			Lua::SetTableValue(l,t);
		}
	}));
	defCParticleSystem.def("GetAlphaMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,umath::to_integral(hComponent->GetAlphaMode()));
	}));
	defCParticleSystem.def("SetAlphaMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t alphaMode) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetAlphaMode(static_cast<pragma::AlphaMode>(alphaMode));
	}));
	defCParticleSystem.def("GetEmissionRate",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetEmissionRate());
	}));
	defCParticleSystem.def("SetEmissionRate",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t emissionRate) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetEmissionRate(emissionRate);
	}));
	defCParticleSystem.def("SetNextParticleEmissionCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t count) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetNextParticleEmissionCount(count);
	}));
	defCParticleSystem.def("PauseEmission",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->PauseEmission();
	}));
	defCParticleSystem.def("ResumeEmission",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ResumeEmission();
	}));
	defCParticleSystem.def("GetLifeTime",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetLifeTime());
	}));
	defCParticleSystem.def("GetSoftParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetSoftParticles());
	}));
	defCParticleSystem.def("SetSoftParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool bSoft) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetSoftParticles(bSoft);
	}));
	defCParticleSystem.def("Simulate",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float tDelta) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Simulate(tDelta);
	}));
	defCParticleSystem.def("Render",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::shared_ptr<prosper::CommandBuffer>&,std::shared_ptr<::Scene>&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,std::shared_ptr<prosper::CommandBuffer> &drawCmd,std::shared_ptr<::Scene> &scene,bool bBloom) {
		pragma::Lua::check_component(l,hComponent);
		if(drawCmd->IsPrimary() == false)
			return;
		hComponent->Render(std::static_pointer_cast<prosper::PrimaryCommandBuffer>(drawCmd),*scene,bBloom);
	}));
	defCParticleSystem.def("GetRenderParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetRenderParticleCount());
	}));
	defCParticleSystem.def("IsActiveOrPaused",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsActiveOrPaused());
	}));
	defCParticleSystem.def("IsEmissionPaused",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsEmissionPaused());
	}));
	defCParticleSystem.def("SetParent",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hParent) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hParent);
		hComponent->SetParent(hParent.get());
	}));
	defCParticleSystem.def("RemoveChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hChild) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hChild);
		hComponent->RemoveChild(hChild.get());
	}));
	defCParticleSystem.def("HasChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hChild) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hChild);
		Lua::PushBool(l,hComponent->HasChild(*hChild));
	}));
	defCParticleSystem.def("GetParent",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *hParent = hComponent->GetParent();
		if(hParent == nullptr)
			return;
		Lua::Push(l,hParent->GetHandle());
	}));
	defCParticleSystem.def("GetVertexBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &vertexBuffer = hComponent->GetVertexBuffer();
		if(vertexBuffer == nullptr)
			return;
		Lua::Push(l,vertexBuffer);
	}));
	defCParticleSystem.def("GetParticleBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &particleBuffer = hComponent->GetParticleBuffer();
		if(particleBuffer == nullptr)
			return;
		Lua::Push(l,particleBuffer);
	}));
	defCParticleSystem.def("GetAnimationStartBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &animStartBuffer = hComponent->GetAnimationStartBuffer();
		if(animStartBuffer == nullptr)
			return;
		Lua::Push(l,animStartBuffer);
	}));
	defCParticleSystem.def("GetAnimationBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &animBuffer = hComponent->GetAnimationBuffer();
		if(animBuffer == nullptr)
			return;
		Lua::Push(l,animBuffer);
	}));
	defCParticleSystem.def("GetAnimationDescriptorSet",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &animDescSetGroup = hComponent->GetAnimationDescriptorSetGroup();
		if(animDescSetGroup == nullptr)
			return;
		Lua::Push(l,animDescSetGroup);
	}));
	defCParticleSystem.def("IsAnimated",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsAnimated());
	}));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_ALIGNED",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Aligned));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_UPRIGHT",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Upright));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_STATIC",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Static));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_WORLD",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::World));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_BILLBOARD",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Billboard));

	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE",umath::to_integral(pragma::AlphaMode::Additive));
	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE_FULL",umath::to_integral(pragma::AlphaMode::AdditiveFull));
	defCParticleSystem.add_static_constant("ALPHA_MODE_OPAQUE",umath::to_integral(pragma::AlphaMode::Opaque));
	defCParticleSystem.add_static_constant("ALPHA_MODE_MASKED",umath::to_integral(pragma::AlphaMode::Masked));
	defCParticleSystem.add_static_constant("ALPHA_MODE_TRANSLUCENT",umath::to_integral(pragma::AlphaMode::Translucent));
	defCParticleSystem.add_static_constant("ALPHA_MODE_PREMULTIPLIED",umath::to_integral(pragma::AlphaMode::Premultiplied));
	defCParticleSystem.add_static_constant("ALPHA_MODE_COUNT",umath::to_integral(pragma::AlphaMode::Count));
	entsMod[defCParticleSystem];

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

	auto defCWorld = luabind::class_<CWorldHandle,BaseEntityComponentHandle>("WorldComponent");
	Lua::register_base_world_component_methods<luabind::class_<CWorldHandle,BaseEntityComponentHandle>,CWorldHandle>(l,defCWorld);
	entsMod[defCWorld];

	auto defCFlashlight = luabind::class_<CFlashlightHandle,BaseEntityComponentHandle>("FlashlightComponent");
	Lua::register_base_flashlight_component_methods<luabind::class_<CFlashlightHandle,BaseEntityComponentHandle>,CFlashlightHandle>(l,defCFlashlight);
	entsMod[defCFlashlight];

	auto defCEnvSoundProbe = luabind::class_<CEnvSoundProbeHandle,BaseEntityComponentHandle>("EnvSoundProbeComponent");
	entsMod[defCEnvSoundProbe];

	auto defCWeather = luabind::class_<CWeatherHandle,BaseEntityComponentHandle>("WeatherComponent");
	Lua::register_base_env_weather_component_methods<luabind::class_<CWeatherHandle,BaseEntityComponentHandle>,CWeatherHandle>(l,defCWeather);
	entsMod[defCWeather];

	auto defCWaterSurface = luabind::class_<CWaterSurfaceHandle,BaseEntityComponentHandle>("WaterSurfaceComponent");
	entsMod[defCWaterSurface];

	auto defCListener = luabind::class_<CListenerHandle,BaseEntityComponentHandle>("ListenerComponent");
	entsMod[defCListener];

	auto defCViewBody = luabind::class_<CViewBodyHandle,BaseEntityComponentHandle>("ViewBodyComponent");
	entsMod[defCViewBody];

	auto defCViewModel = luabind::class_<CViewModelHandle,BaseEntityComponentHandle>("ViewModelComponent");
	entsMod[defCViewModel];

	auto defCFlex = luabind::class_<CFlexHandle,BaseEntityComponentHandle>("FlexComponent");
	Lua::register_base_flex_component_methods<luabind::class_<CFlexHandle,BaseEntityComponentHandle>,CFlexHandle>(l,defCFlex);
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,uint32_t,float)>(&Lua::Flex::SetFlexController));
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,const std::string&,float)>(&Lua::Flex::SetFlexController));
	defCFlex.def("GetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,uint32_t)>(&Lua::Flex::GetFlexController));
	defCFlex.def("GetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,const std::string&)>(&Lua::Flex::GetFlexController));
	defCFlex.def("CalcFlexValue",&Lua::Flex::CalcFlexValue);
	entsMod[defCFlex];

	auto defCSoftBody = luabind::class_<CSoftBodyHandle,BaseEntityComponentHandle>("SoftBodyComponent");
	entsMod[defCSoftBody];

	auto defCVertexAnimated = luabind::class_<CVertexAnimatedHandle,BaseEntityComponentHandle>("VertexAnimatedComponent");
	defCVertexAnimated.def("UpdateVertexAnimationBuffer",static_cast<void(*)(lua_State*,CVertexAnimatedHandle&,std::shared_ptr<prosper::CommandBuffer>&)>([](lua_State *l,CVertexAnimatedHandle &hAnim,std::shared_ptr<prosper::CommandBuffer> &drawCmd) {
		pragma::Lua::check_component(l,hAnim);
		if(drawCmd->IsPrimary() == false)
			return;
		hAnim->UpdateVertexAnimationBuffer(std::static_pointer_cast<prosper::PrimaryCommandBuffer>(drawCmd));
	}));
	defCVertexAnimated.def("GetVertexAnimationBuffer",static_cast<void(*)(lua_State*,CVertexAnimatedHandle&)>([](lua_State *l,CVertexAnimatedHandle &hAnim) {
		pragma::Lua::check_component(l,hAnim);
		auto &buffer = hAnim->GetVertexAnimationBuffer();
		if(buffer == nullptr)
			return;
		Lua::Push<std::shared_ptr<prosper::Buffer>>(l,buffer);
	}));
	defCVertexAnimated.def("GetVertexAnimationBufferMeshOffset",static_cast<void(*)(lua_State*,CVertexAnimatedHandle&,std::shared_ptr<::ModelSubMesh>&)>([](lua_State *l,CVertexAnimatedHandle &hAnim,std::shared_ptr<::ModelSubMesh> &subMesh) {
		pragma::Lua::check_component(l,hAnim);
		uint32_t offset;
		uint32_t animCount;
		auto b = hAnim->GetVertexAnimationBufferMeshOffset(static_cast<CModelSubMesh&>(*subMesh),offset,animCount);
		if(b == false)
			return;
		Lua::PushInt(l,offset);
		Lua::PushInt(l,animCount);
	}));
	defCVertexAnimated.def("GetLocalVertexPosition",static_cast<void(*)(lua_State*,CVertexAnimatedHandle&,std::shared_ptr<::ModelSubMesh>&,uint32_t)>([](lua_State *l,CVertexAnimatedHandle &hAnim,std::shared_ptr<::ModelSubMesh> &subMesh,uint32_t vertexId) {
		pragma::Lua::check_component(l,hAnim);
		Vector3 pos;
		auto b = hAnim->GetLocalVertexPosition(static_cast<CModelSubMesh&>(*subMesh),vertexId,pos);
		if(b == false)
			return;
		Lua::Push<Vector3>(l,pos);
	}));
	entsMod[defCVertexAnimated];

	
	auto defCBSPLeaf = luabind::class_<CBSPLeafHandle,BaseEntityComponentHandle>("BSPLeafComponent");
	entsMod[defCBSPLeaf];

	auto defCIo = luabind::class_<CIOHandle,BaseEntityComponentHandle>("IOComponent");
	Lua::register_base_io_component_methods<luabind::class_<CIOHandle,BaseEntityComponentHandle>,CIOHandle>(l,defCIo);
	entsMod[defCIo];

	auto defCModel = luabind::class_<CModelHandle,BaseEntityComponentHandle>("ModelComponent");
	Lua::register_base_model_component_methods<luabind::class_<CModelHandle,BaseEntityComponentHandle>,CModelHandle>(l,defCModel);
	entsMod[defCModel];

	auto defCAnimated = luabind::class_<CAnimatedHandle,BaseEntityComponentHandle>("AnimatedComponent");
	Lua::register_base_animated_component_methods<luabind::class_<CAnimatedHandle,BaseEntityComponentHandle>,CAnimatedHandle>(l,defCAnimated);
	defCAnimated.def("GetEffectiveBoneTransform",static_cast<void(*)(lua_State*,CAnimatedHandle&,uint32_t)>([](lua_State *l,CAnimatedHandle &hAnim,uint32_t boneIdx) {
		pragma::Lua::check_component(l,hAnim);
		auto &transforms = hAnim->GetProcessedBones();
		if(boneIdx >= transforms.size())
			return;
		Lua::Push<boost::reference_wrapper<Transform>>(l,boost::ref<Transform>(transforms.at(boneIdx)));
	}));
	defCAnimated.def("GetBoneBuffer",static_cast<void(*)(lua_State*,CAnimatedHandle&)>([](lua_State *l,CAnimatedHandle &hAnim) {
		pragma::Lua::check_component(l,hAnim);
		auto *pAnimComponent = hAnim.get();
		if(pAnimComponent == nullptr)
			return;
		auto buf = pAnimComponent->GetBoneBuffer();
		if(buf.expired())
			return;
		Lua::Push(l,buf.lock());
	}));
	defCAnimated.def("GetBoneRenderMatrices",static_cast<void(*)(lua_State*,CAnimatedHandle&)>([](lua_State *l,CAnimatedHandle &hAnim) {
		pragma::Lua::check_component(l,hAnim);
		auto *pAnimComponent = hAnim.get();
		if(pAnimComponent == nullptr)
			return;
		auto &mats = pAnimComponent->GetBoneMatrices();
		auto t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto &m : mats)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<Mat4>(l,m);
			Lua::SetTableValue(l,t);
		}
	}));
	defCAnimated.def("GetBoneRenderMatrix",static_cast<void(*)(lua_State*,CAnimatedHandle&,uint32_t)>([](lua_State *l,CAnimatedHandle &hAnim,uint32_t boneIndex) {
		pragma::Lua::check_component(l,hAnim);
		auto *pAnimComponent = hAnim.get();
		if(pAnimComponent == nullptr)
			return;
		auto &mats = pAnimComponent->GetBoneMatrices();
		if(boneIndex >= mats.size())
			return;
		auto &m = mats.at(boneIndex);
		Lua::Push<Mat4>(l,m);
	}));
	defCAnimated.def("GetLocalVertexPosition",static_cast<void(*)(lua_State*,CVertexAnimatedHandle&,std::shared_ptr<::ModelSubMesh>&,uint32_t)>([](lua_State *l,CVertexAnimatedHandle &hAnim,std::shared_ptr<::ModelSubMesh> &subMesh,uint32_t vertexId) {
		pragma::Lua::check_component(l,hAnim);
		Vector3 pos;
		auto b = hAnim->GetLocalVertexPosition(static_cast<CModelSubMesh&>(*subMesh),vertexId,pos);
		if(b == false)
			return;
		Lua::Push<Vector3>(l,pos);
	}));
	defCAnimated.add_static_constant("EVENT_ON_SKELETON_UPDATED",pragma::CAnimatedComponent::EVENT_ON_SKELETON_UPDATED);
	defCAnimated.add_static_constant("EVENT_ON_BONE_MATRICES_UPDATED",pragma::CAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED);
	defCAnimated.add_static_constant("EVENT_ON_BONE_BUFFER_INITIALIZED",pragma::CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED);
	entsMod[defCAnimated];

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

	auto defCLightMap = luabind::class_<CLightMapHandle,BaseEntityComponentHandle>("LightMapComponent");
	entsMod[defCLightMap];

	auto defCGeneric = luabind::class_<CGenericHandle,BaseEntityComponentHandle>("EntityComponent");
	//Lua::register_base_generic_component_methods<luabind::class_<CGenericHandle,BaseEntityComponentHandle>,CGenericHandle>(l,defCGeneric);
	entsMod[defCGeneric];
}

void Lua::Render::GetModelMatrix(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Mat4 mat = hEnt->GetModelMatrix();
	luabind::object(l,mat).push(l);
}

void Lua::Render::GetTranslationMatrix(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Mat4 mat = hEnt->GetTranslationMatrix();
	luabind::object(l,mat).push(l);
}

void Lua::Render::GetRotationMatrix(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Mat4 mat = hEnt->GetRotationMatrix();
	luabind::object(l,mat).push(l);
}

void Lua::Render::GetTransformationMatrix(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Mat4 mat = hEnt->GetTransformationMatrix();
	luabind::object(l,mat).push(l);
}

void Lua::Render::SetRenderMode(lua_State *l,CRenderHandle &hEnt,unsigned int mode)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetRenderMode(RenderMode(mode));
}

void Lua::Render::GetRenderMode(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	RenderMode mode = hEnt->GetRenderMode();
	Lua::PushInt(l,static_cast<int>(mode));
}

void Lua::Render::GetRenderSphereBounds(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto sphere = hEnt->GetRenderSphereBounds();
	Lua::Push<Vector3>(l,sphere.pos);
	Lua::PushNumber(l,sphere.radius);
}

void Lua::Render::GetRenderBounds(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Vector3 min;
	Vector3 max;
	hEnt->GetRenderBounds(&min,&max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}

void Lua::Render::SetRenderBounds(lua_State *l,CRenderHandle &hEnt,Vector3 &min,Vector3 &max)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetRenderBounds(min,max);
}

void Lua::Render::UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::CommandBuffer> &drawCmd,bool bForceBufferUpdate)
{
	pragma::Lua::check_component(l,hEnt);
	if(drawCmd->IsPrimary() == false)
		return;
	hEnt->UpdateRenderData(std::static_pointer_cast<prosper::PrimaryCommandBuffer>(drawCmd),bForceBufferUpdate);
}
void Lua::Render::UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::CommandBuffer> &drawCmd) {UpdateRenderBuffers(l,hEnt,drawCmd,false);}
void Lua::Render::GetRenderBuffer(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto buf = hEnt->GetRenderBuffer();
	if(buf.expired())
		return;
	Lua::Push(l,buf.lock());
}
void Lua::Render::GetBoneBuffer(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto *pAnimComponent = static_cast<pragma::CAnimatedComponent*>(hEnt->GetEntity().GetAnimatedComponent().get());
	if(pAnimComponent == nullptr)
		return;
	auto buf = pAnimComponent->GetBoneBuffer();
	if(buf.expired())
		return;
	Lua::Push(l,buf.lock());
}

//////////////

void Lua::Flex::SetFlexController(lua_State *l,CFlexHandle &hEnt,uint32_t flexId,float cycle)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetFlexController(flexId,cycle);
}
void Lua::Flex::SetFlexController(lua_State *l,CFlexHandle &hEnt,const std::string &flexController,float cycle)
{
	pragma::Lua::check_component(l,hEnt);
	auto flexId = 0u;
	auto mdlComponent = hEnt->GetEntity().GetModelComponent();
	if(mdlComponent.expired() || mdlComponent->LookupFlexController(flexController,flexId) == false)
		return;
	hEnt->SetFlexController(flexId,cycle);
}
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
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::CheckString(l,-3);
		std::string val = Lua::ParticleSystem::get_key_value(l,-2);
		StringToLower(key);
		values[key] = val;
	}
	hComponent.AddInitializer(name,values);
}
void Lua::ParticleSystem::AddOperator(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::CheckString(l,-3);
		std::string val = Lua::ParticleSystem::get_key_value(l,-2);
		StringToLower(key);
		values[key] = val;
	}
	hComponent.AddOperator(name,values);
}
void Lua::ParticleSystem::AddRenderer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::CheckString(l,-3);
		std::string val = Lua::ParticleSystem::get_key_value(l,-2);
		StringToLower(key);
		values[key] = val;
	}
	hComponent.AddRenderer(name,values);
}
