/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/physics/raytraces.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/lua/classes/ldef_angle.h>
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/netpacket.hpp>

namespace Lua
{
	namespace SoundEmitter
	{
		static void CreateSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,bool bTransmit);
		static void EmitSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit);
	};
};
void SGame::RegisterLuaEntityComponent(luabind::class_<BaseEntityComponentHandleWrapper> &def)
{
	Game::RegisterLuaEntityComponent(def);
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,uint32_t,NetPacket&,pragma::networking::TargetRecipientFilter&)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t protocol,uint32_t eventId,NetPacket &packet,pragma::networking::TargetRecipientFilter &rf) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,packet,static_cast<pragma::networking::Protocol>(protocol),rf);
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,uint32_t,NetPacket&)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t protocol,uint32_t eventId,NetPacket &packet) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,packet,static_cast<pragma::networking::Protocol>(protocol));
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,uint32_t)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t protocol,uint32_t eventId) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,static_cast<pragma::networking::Protocol>(protocol));
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,NetPacket&)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId,NetPacket &packet) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,packet);
	}));
}
void RegisterLuaEntityComponents2(lua_State *l,luabind::module_ &entsMod);
void SGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);

	auto *l = GetLuaState();
	Lua::register_sv_ai_component(l,entsMod);
	Lua::register_sv_character_component(l,entsMod);
	Lua::register_sv_player_component(l,entsMod);
	Lua::register_sv_vehicle_component(l,entsMod);
	Lua::register_sv_weapon_component(l,entsMod);

	auto defSGamemode = luabind::class_<SGamemodeHandle,BaseEntityComponentHandle>("GamemodeComponent");
	Lua::register_base_gamemode_component_methods<luabind::class_<SGamemodeHandle,BaseEntityComponentHandle>,SGamemodeHandle>(l,defSGamemode);
	entsMod[defSGamemode];

	auto defSColor = luabind::class_<SColorHandle,BaseEntityComponentHandle>("ColorComponent");
	Lua::register_base_color_component_methods<luabind::class_<SColorHandle,BaseEntityComponentHandle>,SColorHandle>(l,defSColor);
	entsMod[defSColor];

	auto defSScore = luabind::class_<SScoreHandle,BaseEntityComponentHandle>("ScoreComponent");
	Lua::register_base_score_component_methods<luabind::class_<SScoreHandle,BaseEntityComponentHandle>,SScoreHandle>(l,defSScore);
	entsMod[defSScore];

	auto defSFlammable = luabind::class_<SFlammableHandle,BaseEntityComponentHandle>("FlammableComponent");
	Lua::register_base_flammable_component_methods<luabind::class_<SFlammableHandle,BaseEntityComponentHandle>,SFlammableHandle>(l,defSFlammable);
	entsMod[defSFlammable];

	auto defSHealth = luabind::class_<SHealthHandle,BaseEntityComponentHandle>("HealthComponent");
	Lua::register_base_health_component_methods<luabind::class_<SHealthHandle,BaseEntityComponentHandle>,SHealthHandle>(l,defSHealth);
	entsMod[defSHealth];

	auto defSName = luabind::class_<SNameHandle,BaseEntityComponentHandle>("NameComponent");
	Lua::register_base_name_component_methods<luabind::class_<SNameHandle,BaseEntityComponentHandle>,SNameHandle>(l,defSName);
	entsMod[defSName];

	auto defSNetworked = luabind::class_<SNetworkedHandle,BaseEntityComponentHandle>("NetworkedComponent");
	Lua::register_base_networked_component_methods<luabind::class_<SNetworkedHandle,BaseEntityComponentHandle>,SNetworkedHandle>(l,defSNetworked);
	entsMod[defSNetworked];

	auto defSObservable = luabind::class_<SObservableHandle,BaseEntityComponentHandle>("ObservableComponent");
	Lua::register_base_observable_component_methods<luabind::class_<SObservableHandle,BaseEntityComponentHandle>,SObservableHandle>(l,defSObservable);
	entsMod[defSObservable];

	auto defSShooter = luabind::class_<SShooterHandle,BaseEntityComponentHandle>("ShooterComponent");
	Lua::register_base_shooter_component_methods<luabind::class_<SShooterHandle,BaseEntityComponentHandle>,SShooterHandle>(l,defSShooter);
	entsMod[defSShooter];

	auto defSPhysics = luabind::class_<SPhysicsHandle,BaseEntityComponentHandle>("PhysicsComponent");
	Lua::register_base_physics_component_methods<luabind::class_<SPhysicsHandle,BaseEntityComponentHandle>,SPhysicsHandle>(l,defSPhysics);
	entsMod[defSPhysics];

	auto defSRadius = luabind::class_<SRadiusHandle,BaseEntityComponentHandle>("RadiusComponent");
	Lua::register_base_radius_component_methods<luabind::class_<SRadiusHandle,BaseEntityComponentHandle>,SRadiusHandle>(l,defSRadius);
	entsMod[defSRadius];

	auto defSRender = luabind::class_<SRenderHandle,BaseEntityComponentHandle>("RenderComponent");
	Lua::register_base_render_component_methods<luabind::class_<SRenderHandle,BaseEntityComponentHandle>,SRenderHandle>(l,defSRender);
	entsMod[defSRender];

	auto defSSoundEmitter = luabind::class_<SSoundEmitterHandle,BaseEntityComponentHandle>("SoundEmitterComponent");
	Lua::register_base_sound_emitter_component_methods<luabind::class_<SSoundEmitterHandle,BaseEntityComponentHandle>,SSoundEmitterHandle>(l,defSSoundEmitter);
	defSSoundEmitter.def("CreateSound",static_cast<void(*)(lua_State*,SSoundEmitterHandle&,std::string,uint32_t,bool)>(&Lua::SoundEmitter::CreateSound));
	defSSoundEmitter.def("EmitSound",static_cast<void(*)(lua_State*,SSoundEmitterHandle&,std::string,uint32_t,float,float,bool)>(&Lua::SoundEmitter::EmitSound));
	entsMod[defSSoundEmitter];

	auto defSToggle = luabind::class_<SToggleHandle,BaseEntityComponentHandle>("ToggleComponent");
	Lua::register_base_toggle_component_methods<luabind::class_<SToggleHandle,BaseEntityComponentHandle>,SToggleHandle>(l,defSToggle);
	entsMod[defSToggle];

	auto defSTransform = luabind::class_<STransformHandle,BaseEntityComponentHandle>("TransformComponent");
	Lua::register_base_transform_component_methods<luabind::class_<STransformHandle,BaseEntityComponentHandle>,STransformHandle>(l,defSTransform);
	entsMod[defSTransform];

	auto defSWheel = luabind::class_<SWheelHandle,BaseEntityComponentHandle>("WheelComponent");
	Lua::register_base_wheel_component_methods<luabind::class_<SWheelHandle,BaseEntityComponentHandle>,SWheelHandle>(l,defSWheel);
	entsMod[defSWheel];

	auto defSSoundDsp = luabind::class_<SSoundDspHandle,BaseEntityComponentHandle>("SoundDspComponent");
	Lua::register_base_env_sound_dsp_component_methods<luabind::class_<SSoundDspHandle,BaseEntityComponentHandle>,SSoundDspHandle>(l,defSSoundDsp);
	entsMod[defSSoundDsp];

	auto defSSoundDspChorus = luabind::class_<SSoundDspChorusHandle,BaseEntityComponentHandle>("SoundDspChorusComponent");
	entsMod[defSSoundDspChorus];

	auto defSSoundDspDistortion = luabind::class_<SSoundDspDistortionHandle,BaseEntityComponentHandle>("SoundDspDistortionComponent");
	entsMod[defSSoundDspDistortion];

	auto defSSoundDspEAXReverb = luabind::class_<SSoundDspEAXReverbHandle,BaseEntityComponentHandle>("SoundDspEAXReverbComponent");
	entsMod[defSSoundDspEAXReverb];

	auto defSSoundDspEcho = luabind::class_<SSoundDspEchoHandle,BaseEntityComponentHandle>("SoundDspEchoComponent");
	entsMod[defSSoundDspEcho];

	auto defSSoundDspEqualizer = luabind::class_<SSoundDspEqualizerHandle,BaseEntityComponentHandle>("SoundDspEqualizerComponent");
	entsMod[defSSoundDspEqualizer];

	auto defSSoundDspFlanger = luabind::class_<SSoundDspFlangerHandle,BaseEntityComponentHandle>("SoundDspFlangerComponent");
	entsMod[defSSoundDspFlanger];

	auto defSCamera = luabind::class_<SCameraHandle,BaseEntityComponentHandle>("CameraComponent");
	Lua::register_base_env_camera_component_methods<luabind::class_<SCameraHandle,BaseEntityComponentHandle>,SCameraHandle>(l,defSCamera);
	entsMod[defSCamera];

	auto defSDecal = luabind::class_<SDecalHandle,BaseEntityComponentHandle>("DecalComponent");
	Lua::register_base_decal_component_methods<luabind::class_<SDecalHandle,BaseEntityComponentHandle>,SDecalHandle>(l,defSDecal);
	entsMod[defSDecal];

	auto defSExplosion = luabind::class_<SExplosionHandle,BaseEntityComponentHandle>("ExplosionComponent");
	Lua::register_base_env_explosion_component_methods<luabind::class_<SExplosionHandle,BaseEntityComponentHandle>,SExplosionHandle>(l,defSExplosion);
	entsMod[defSExplosion];

	auto defSFire = luabind::class_<SFireHandle,BaseEntityComponentHandle>("FireComponent");
	Lua::register_base_env_fire_component_methods<luabind::class_<SFireHandle,BaseEntityComponentHandle>,SFireHandle>(l,defSFire);
	entsMod[defSFire];

	auto defSFogController = luabind::class_<SFogControllerHandle,BaseEntityComponentHandle>("FogControllerComponent");
	Lua::register_base_env_fog_controller_component_methods<luabind::class_<SFogControllerHandle,BaseEntityComponentHandle>,SFogControllerHandle>(l,defSFogController);
	entsMod[defSFogController];

	auto defSLight = luabind::class_<SLightHandle,BaseEntityComponentHandle>("LightComponent");
	Lua::register_base_env_light_component_methods<luabind::class_<SLightHandle,BaseEntityComponentHandle>,SLightHandle>(l,defSLight);
	entsMod[defSLight];

	auto defSLightDirectional = luabind::class_<SLightDirectionalHandle,BaseEntityComponentHandle>("LightDirectionalComponent");
	Lua::register_base_env_light_directional_component_methods<luabind::class_<SLightDirectionalHandle,BaseEntityComponentHandle>,SLightDirectionalHandle>(l,defSLightDirectional);
	entsMod[defSLightDirectional];

	auto defSLightPoint = luabind::class_<SLightPointHandle,BaseEntityComponentHandle>("LightPointComponent");
	Lua::register_base_env_light_point_component_methods<luabind::class_<SLightPointHandle,BaseEntityComponentHandle>,SLightPointHandle>(l,defSLightPoint);
	entsMod[defSLightPoint];

	auto defSLightSpot = luabind::class_<SLightSpotHandle,BaseEntityComponentHandle>("LightSpotComponent");
	Lua::register_base_env_light_spot_component_methods<luabind::class_<SLightSpotHandle,BaseEntityComponentHandle>,SLightSpotHandle>(l,defSLightSpot);
	entsMod[defSLightSpot];

	auto defSLightSpotVol = luabind::class_<SLightSpotVolHandle,BaseEntityComponentHandle>("LightSpotVolComponent");
	Lua::register_base_env_light_spot_vol_component_methods<luabind::class_<SLightSpotVolHandle,BaseEntityComponentHandle>,SLightSpotVolHandle>(l,defSLightSpotVol);
	entsMod[defSLightSpotVol];

	auto defSMicrophone = luabind::class_<SMicrophoneHandle,BaseEntityComponentHandle>("MicrophoneComponent");
	Lua::register_base_env_microphone_component_methods<luabind::class_<SMicrophoneHandle,BaseEntityComponentHandle>,SMicrophoneHandle>(l,defSMicrophone);
	entsMod[defSMicrophone];

	auto defSParticleSystem = luabind::class_<SParticleSystemHandle,BaseEntityComponentHandle>("ParticleSystemComponent");
	Lua::register_base_env_particle_system_component_methods<luabind::class_<SParticleSystemHandle,BaseEntityComponentHandle>,SParticleSystemHandle>(l,defSParticleSystem);
	entsMod[defSParticleSystem];

	auto defSQuake = luabind::class_<SQuakeHandle,BaseEntityComponentHandle>("QuakeComponent");
	Lua::register_base_env_quake_component_methods<luabind::class_<SQuakeHandle,BaseEntityComponentHandle>,SQuakeHandle>(l,defSQuake);
	entsMod[defSQuake];

	auto defSSmokeTrail = luabind::class_<SSmokeTrailHandle,BaseEntityComponentHandle>("SmokeTrailComponent");
	Lua::register_base_env_smoke_trail_component_methods<luabind::class_<SSmokeTrailHandle,BaseEntityComponentHandle>,SSmokeTrailHandle>(l,defSSmokeTrail);
	entsMod[defSSmokeTrail];

	auto defSSound = luabind::class_<SSoundHandle,BaseEntityComponentHandle>("SoundComponent");
	Lua::register_base_env_sound_component_methods<luabind::class_<SSoundHandle,BaseEntityComponentHandle>,SSoundHandle>(l,defSSound);
	entsMod[defSSound];

	auto defSSoundScape = luabind::class_<SSoundScapeHandle,BaseEntityComponentHandle>("SoundScapeComponent");
	Lua::register_base_env_soundscape_component_methods<luabind::class_<SSoundScapeHandle,BaseEntityComponentHandle>,SSoundScapeHandle>(l,defSSoundScape);
	entsMod[defSSoundScape];

	auto defSSprite = luabind::class_<SSpriteHandle,BaseEntityComponentHandle>("SpriteComponent");
	Lua::register_base_env_sprite_component_methods<luabind::class_<SSpriteHandle,BaseEntityComponentHandle>,SSpriteHandle>(l,defSSprite);
	entsMod[defSSprite];

	auto defSTimescale = luabind::class_<SEnvTimescaleHandle,BaseEntityComponentHandle>("EnvTimeScaleComponent");
	Lua::register_base_env_timescale_component_methods<luabind::class_<SEnvTimescaleHandle,BaseEntityComponentHandle>,SEnvTimescaleHandle>(l,defSTimescale);
	entsMod[defSTimescale];

	auto defSWind = luabind::class_<SWindHandle,BaseEntityComponentHandle>("WindComponent");
	Lua::register_base_env_wind_component_methods<luabind::class_<SWindHandle,BaseEntityComponentHandle>,SWindHandle>(l,defSWind);
	entsMod[defSWind];

	auto defSFilterClass = luabind::class_<SFilterClassHandle,BaseEntityComponentHandle>("FilterClassComponent");
	Lua::register_base_env_filter_class_component_methods<luabind::class_<SFilterClassHandle,BaseEntityComponentHandle>,SFilterClassHandle>(l,defSFilterClass);
	entsMod[defSFilterClass];

	auto defSFilterName = luabind::class_<SFilterNameHandle,BaseEntityComponentHandle>("FilterNameComponent");
	Lua::register_base_env_filter_name_component_methods<luabind::class_<SFilterNameHandle,BaseEntityComponentHandle>,SFilterNameHandle>(l,defSFilterName);
	entsMod[defSFilterName];

	auto defSBrush = luabind::class_<SBrushHandle,BaseEntityComponentHandle>("BrushComponent");
	Lua::register_base_func_brush_component_methods<luabind::class_<SBrushHandle,BaseEntityComponentHandle>,SBrushHandle>(l,defSBrush);
	entsMod[defSBrush];

	auto defSKinematic = luabind::class_<SKinematicHandle,BaseEntityComponentHandle>("KinematicComponent");
	Lua::register_base_func_kinematic_component_methods<luabind::class_<SKinematicHandle,BaseEntityComponentHandle>,SKinematicHandle>(l,defSKinematic);
	entsMod[defSKinematic];

	auto defSFuncPhysics = luabind::class_<SFuncPhysicsHandle,BaseEntityComponentHandle>("FuncPhysicsComponent");
	Lua::register_base_func_physics_component_methods<luabind::class_<SFuncPhysicsHandle,BaseEntityComponentHandle>,SFuncPhysicsHandle>(l,defSFuncPhysics);
	entsMod[defSFuncPhysics];

	auto defSFuncSoftPhysics = luabind::class_<SFuncSoftPhysicsHandle,BaseEntityComponentHandle>("FuncSoftPhysicsComponent");
	Lua::register_base_func_soft_physics_component_methods<luabind::class_<SFuncSoftPhysicsHandle,BaseEntityComponentHandle>,SFuncSoftPhysicsHandle>(l,defSFuncSoftPhysics);
	entsMod[defSFuncSoftPhysics];

	auto defSFuncPortal = luabind::class_<SFuncPortalHandle,BaseEntityComponentHandle>("FuncPortalComponent");
	Lua::register_base_func_portal_component_methods<luabind::class_<SFuncPortalHandle,BaseEntityComponentHandle>,SFuncPortalHandle>(l,defSFuncPortal);
	entsMod[defSFuncPortal];

	auto defSWater = luabind::class_<SWaterHandle,BaseEntityComponentHandle>("WaterComponent");
	Lua::register_base_func_water_component_methods<luabind::class_<SWaterHandle,BaseEntityComponentHandle>,SWaterHandle>(l,defSWater);
	entsMod[defSWater];

	auto defSButton = luabind::class_<SButtonHandle,BaseEntityComponentHandle>("ButtonComponent");
	Lua::register_base_func_button_component_methods<luabind::class_<SButtonHandle,BaseEntityComponentHandle>,SButtonHandle>(l,defSButton);
	entsMod[defSButton];

	auto defSPlayerSpawn = luabind::class_<SPlayerSpawnHandle,BaseEntityComponentHandle>("PlayerSpawnComponent");
	entsMod[defSPlayerSpawn];

	auto defSLogicRelay = luabind::class_<SLogicRelayHandle,BaseEntityComponentHandle>("LogicRelayComponent");
	Lua::register_base_logic_relay_component_methods<luabind::class_<SLogicRelayHandle,BaseEntityComponentHandle>,SLogicRelayHandle>(l,defSLogicRelay);
	entsMod[defSLogicRelay];

	auto defSBot = luabind::class_<SBotHandle,BaseEntityComponentHandle>("BotComponent");
	Lua::register_base_bot_component_methods<luabind::class_<SBotHandle,BaseEntityComponentHandle>,SBotHandle>(l,defSBot);
	entsMod[defSBot];

	RegisterLuaEntityComponents2(l,entsMod); // Split up to prevent compiler errors
}

void Lua::SoundEmitter::CreateSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->CreateSound(sndname,static_cast<ALSoundType>(soundType),bTransmit);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
void Lua::SoundEmitter::EmitSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch,bTransmit);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
