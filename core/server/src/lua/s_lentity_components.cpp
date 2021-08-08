/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/entities/components/s_sound_emitter_component.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_color_component.hpp"
#include "pragma/entities/components/s_score_component.hpp"
#include "pragma/entities/components/s_flammable_component.hpp"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/entities/components/s_name_component.hpp"
#include "pragma/entities/components/s_networked_component.hpp"
#include "pragma/entities/components/s_observable_component.hpp"
#include "pragma/entities/components/s_physics_component.hpp"
#include "pragma/entities/components/s_radius_component.hpp"
#include "pragma/entities/components/s_render_component.hpp"
#include "pragma/entities/components/s_sound_emitter_component.hpp"
#include "pragma/entities/components/s_toggle_component.hpp"
#include "pragma/entities/components/s_transform_component.hpp"
#include "pragma/entities/components/s_vehicle_component.hpp"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/entities/components/s_wheel_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/environment/audio/s_env_sound_dsp.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_equalizer.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_flanger.h"
#include "pragma/entities/environment/s_env_camera.h"
#include "pragma/entities/environment/effects/s_env_explosion.h"
#include "pragma/entities/environment/effects/s_env_fire.h"
#include "pragma/entities/environment/s_env_decal.h"
#include "pragma/entities/environment/s_env_fog_controller.h"
#include "pragma/entities/environment/lights/s_env_light.h"
#include "pragma/entities/environment/lights/s_env_light_directional.h"
#include "pragma/entities/environment/lights/s_env_light_point.h"
#include "pragma/entities/environment/lights/s_env_light_spot.h"
#include "pragma/entities/environment/lights/s_env_light_spot_vol.h"
#include "pragma/entities/environment/s_env_microphone.h"
#include "pragma/entities/environment/effects/s_env_particle_system.h"
#include "pragma/entities/environment/s_env_quake.h"
#include "pragma/entities/environment/effects/s_env_smoke_trail.h"
#include "pragma/entities/environment/audio/s_env_sound.h"
#include "pragma/entities/environment/audio/s_env_soundscape.h"
#include "pragma/entities/environment/effects/s_env_sprite.h"
#include "pragma/entities/environment/s_env_wind.hpp"
#include "pragma/entities/filter/s_filter_entity_class.h"
#include "pragma/entities/filter/s_filter_entity_name.h"
#include "pragma/entities/func/s_func_brush.h"
#include "pragma/entities/func/s_func_kinematic.hpp"
#include "pragma/entities/func/s_func_physics.h"
#include "pragma/entities/func/s_func_softphysics.hpp"
#include "pragma/entities/func/s_func_portal.h"
#include "pragma/entities/func/s_func_water.h"
#include "pragma/entities/func/s_funcbutton.h"
#include "pragma/entities/game_player_spawn.h"
#include "pragma/entities/logic/s_logic_relay.h"
#include "pragma/entities/s_bot.h"
#include "pragma/entities/components/s_shooter_component.hpp"
#include "pragma/entities/components/s_entity_component.hpp"
#include "pragma/entities/components/s_time_scale_component.hpp"
#include "pragma/entities/components/s_gamemode_component.hpp"
#include "pragma/entities/environment/s_env_timescale.h"
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
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/netpacket.hpp>

namespace Lua
{
	namespace SoundEmitter
	{
		static void CreateSound(lua_State *l,pragma::SSoundEmitterComponent &hEnt,std::string sndname,uint32_t soundType,bool bTransmit);
		static void EmitSound(lua_State *l,pragma::SSoundEmitterComponent &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit);
	};
};
void SGame::RegisterLuaEntityComponent(luabind::class_<pragma::BaseEntityComponent> &def)
{
	Game::RegisterLuaEntityComponent(def);
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,uint32_t,NetPacket&,pragma::networking::TargetRecipientFilter&)>(
		[](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t protocol,uint32_t eventId,NetPacket &packet,pragma::networking::TargetRecipientFilter &rf) {
			static_cast<SBaseEntity&>(hComponent.GetEntity()).SendNetEvent(eventId,packet,static_cast<pragma::networking::Protocol>(protocol),rf);
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,uint32_t,NetPacket&)>(
		[](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t protocol,uint32_t eventId,NetPacket &packet) {
			static_cast<SBaseEntity&>(hComponent.GetEntity()).SendNetEvent(eventId,packet,static_cast<pragma::networking::Protocol>(protocol));
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,uint32_t)>(
		[](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t protocol,uint32_t eventId) {
			static_cast<SBaseEntity&>(hComponent.GetEntity()).SendNetEvent(eventId,static_cast<pragma::networking::Protocol>(protocol));
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,NetPacket&)>(
		[](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t eventId,NetPacket &packet) {
			static_cast<SBaseEntity&>(hComponent.GetEntity()).SendNetEvent(eventId,packet);
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

	auto defSGamemode = luabind::class_<pragma::SGamemodeComponent,pragma::BaseGamemodeComponent>("GamemodeComponent");
	entsMod[defSGamemode];

	auto defSColor = luabind::class_<pragma::SColorComponent,pragma::BaseColorComponent>("ColorComponent");
	entsMod[defSColor];

	auto defSScore = luabind::class_<pragma::SScoreComponent,pragma::BaseScoreComponent>("ScoreComponent");
	entsMod[defSScore];

	auto defSFlammable = luabind::class_<pragma::SFlammableComponent,pragma::BaseFlammableComponent>("FlammableComponent");
	entsMod[defSFlammable];

	auto defSHealth = luabind::class_<pragma::SHealthComponent,pragma::BaseHealthComponent>("HealthComponent");
	entsMod[defSHealth];

	auto defSName = luabind::class_<pragma::SNameComponent,pragma::BaseNameComponent>("NameComponent");
	entsMod[defSName];

	auto defSNetworked = luabind::class_<pragma::SNetworkedComponent,pragma::BaseNetworkedComponent>("NetworkedComponent");
	entsMod[defSNetworked];

	auto defSObservable = luabind::class_<pragma::SObservableComponent,pragma::BaseObservableComponent>("ObservableComponent");
	entsMod[defSObservable];

	auto defSShooter = luabind::class_<pragma::SShooterComponent,pragma::BaseShooterComponent>("ShooterComponent");
	entsMod[defSShooter];

	auto defSPhysics = luabind::class_<pragma::SPhysicsComponent,pragma::BasePhysicsComponent>("PhysicsComponent");
	entsMod[defSPhysics];

	auto defSRadius = luabind::class_<pragma::SRadiusComponent,pragma::BaseRadiusComponent>("RadiusComponent");
	entsMod[defSRadius];

	auto defSRender = luabind::class_<pragma::SRenderComponent,pragma::BaseRenderComponent>("RenderComponent");
	entsMod[defSRender];

	auto defSSoundEmitter = luabind::class_<pragma::SSoundEmitterComponent,pragma::BaseSoundEmitterComponent>("SoundEmitterComponent");
	defSSoundEmitter.def("CreateSound",static_cast<void(*)(lua_State*,pragma::SSoundEmitterComponent&,std::string,uint32_t,bool)>(&Lua::SoundEmitter::CreateSound));
	defSSoundEmitter.def("EmitSound",static_cast<void(*)(lua_State*,pragma::SSoundEmitterComponent&,std::string,uint32_t,float,float,bool)>(&Lua::SoundEmitter::EmitSound));
	entsMod[defSSoundEmitter];

	auto defSToggle = luabind::class_<pragma::SToggleComponent,pragma::BaseToggleComponent>("ToggleComponent");
	entsMod[defSToggle];

	auto defSTransform = luabind::class_<pragma::STransformComponent,pragma::BaseTransformComponent>("TransformComponent");
	entsMod[defSTransform];

	auto defSWheel = luabind::class_<pragma::SWheelComponent,pragma::BaseWheelComponent>("WheelComponent");
	entsMod[defSWheel];

	auto defSSoundDsp = luabind::class_<pragma::SSoundDspComponent,pragma::BaseEnvSoundDspComponent>("SoundDspComponent");
	entsMod[defSSoundDsp];

	auto defSSoundDspChorus = luabind::class_<pragma::SSoundDspChorusComponent,pragma::BaseEnvSoundDspComponent>("SoundDspChorusComponent");
	entsMod[defSSoundDspChorus];

	auto defSSoundDspDistortion = luabind::class_<pragma::SSoundDspDistortionComponent,pragma::BaseEnvSoundDspComponent>("SoundDspDistortionComponent");
	entsMod[defSSoundDspDistortion];

	auto defSSoundDspEAXReverb = luabind::class_<pragma::SSoundDspEAXReverbComponent,pragma::BaseEnvSoundDspComponent>("SoundDspEAXReverbComponent");
	entsMod[defSSoundDspEAXReverb];

	auto defSSoundDspEcho = luabind::class_<pragma::SSoundDspEchoComponent,pragma::BaseEnvSoundDspComponent>("SoundDspEchoComponent");
	entsMod[defSSoundDspEcho];

	auto defSSoundDspEqualizer = luabind::class_<pragma::SSoundDspEqualizerComponent,pragma::BaseEnvSoundDspComponent>("SoundDspEqualizerComponent");
	entsMod[defSSoundDspEqualizer];

	auto defSSoundDspFlanger = luabind::class_<pragma::SSoundDspFlangerComponent,pragma::BaseEnvSoundDspComponent>("SoundDspFlangerComponent");
	entsMod[defSSoundDspFlanger];

	auto defSCamera = luabind::class_<pragma::SCameraComponent,pragma::BaseEnvCameraComponent>("CameraComponent");
	entsMod[defSCamera];

	auto defSDecal = luabind::class_<pragma::SDecalComponent,pragma::BaseEnvDecalComponent>("DecalComponent");
	entsMod[defSDecal];

	auto defSExplosion = luabind::class_<pragma::SExplosionComponent,pragma::BaseEnvExplosionComponent>("ExplosionComponent");
	entsMod[defSExplosion];

	auto defSFire = luabind::class_<pragma::SFireComponent,pragma::BaseEnvFireComponent>("FireComponent");
	entsMod[defSFire];

	auto defSFogController = luabind::class_<pragma::SFogControllerComponent,pragma::BaseEnvFogControllerComponent>("FogControllerComponent");
	entsMod[defSFogController];

	auto defSLight = luabind::class_<pragma::SLightComponent,pragma::BaseEnvLightComponent>("LightComponent");
	entsMod[defSLight];

	auto defSLightDirectional = luabind::class_<pragma::SLightDirectionalComponent,pragma::BaseEnvLightDirectionalComponent>("LightDirectionalComponent");
	entsMod[defSLightDirectional];

	auto defSLightPoint = luabind::class_<pragma::SLightPointComponent,pragma::BaseEnvLightPointComponent>("LightPointComponent");
	entsMod[defSLightPoint];

	auto defSLightSpot = luabind::class_<pragma::SLightSpotComponent,pragma::BaseEnvLightSpotComponent>("LightSpotComponent");
	entsMod[defSLightSpot];

	auto defSLightSpotVol = luabind::class_<pragma::SLightSpotVolComponent,pragma::BaseEnvLightSpotVolComponent>("LightSpotVolComponent");
	entsMod[defSLightSpotVol];

	auto defSMicrophone = luabind::class_<pragma::SMicrophoneComponent,pragma::BaseEnvMicrophoneComponent>("MicrophoneComponent");
	entsMod[defSMicrophone];

	auto defSParticleSystem = luabind::class_<pragma::SParticleSystemComponent,pragma::BaseEnvParticleSystemComponent>("ParticleSystemComponent");
	entsMod[defSParticleSystem];

	auto defSQuake = luabind::class_<pragma::SQuakeComponent,pragma::BaseEnvQuakeComponent>("QuakeComponent");
	entsMod[defSQuake];

	auto defSSmokeTrail = luabind::class_<pragma::SSmokeTrailComponent,pragma::BaseEnvSmokeTrailComponent>("SmokeTrailComponent");
	entsMod[defSSmokeTrail];

	auto defSSound = luabind::class_<pragma::SSoundComponent,pragma::BaseEnvSoundComponent>("SoundComponent");
	entsMod[defSSound];

	auto defSSoundScape = luabind::class_<pragma::SSoundScapeComponent,pragma::BaseEnvSoundScapeComponent>("SoundScapeComponent");
	entsMod[defSSoundScape];

	auto defSSprite = luabind::class_<pragma::SSpriteComponent,pragma::BaseEnvSpriteComponent>("SpriteComponent");
	entsMod[defSSprite];

	auto defSTimescale = luabind::class_<pragma::SEnvTimescaleComponent,pragma::BaseEnvTimescaleComponent>("EnvTimeScaleComponent");
	entsMod[defSTimescale];

	auto defSWind = luabind::class_<pragma::SWindComponent,pragma::BaseEnvWindComponent>("WindComponent");
	entsMod[defSWind];

	auto defSFilterClass = luabind::class_<pragma::SFilterClassComponent,pragma::BaseFilterClassComponent>("FilterClassComponent");
	entsMod[defSFilterClass];

	auto defSFilterName = luabind::class_<pragma::SFilterNameComponent,pragma::BaseFilterNameComponent>("FilterNameComponent");
	entsMod[defSFilterName];

	auto defSBrush = luabind::class_<pragma::SBrushComponent,pragma::BaseFuncBrushComponent>("BrushComponent");
	entsMod[defSBrush];

	auto defSKinematic = luabind::class_<pragma::SKinematicComponent,pragma::BaseFuncKinematicComponent>("KinematicComponent");
	entsMod[defSKinematic];

	auto defSFuncPhysics = luabind::class_<pragma::SFuncPhysicsComponent,pragma::BaseFuncPhysicsComponent>("FuncPhysicsComponent");
	entsMod[defSFuncPhysics];

	auto defSFuncSoftPhysics = luabind::class_<pragma::SFuncSoftPhysicsComponent,pragma::BaseFuncSoftPhysicsComponent>("FuncSoftPhysicsComponent");
	entsMod[defSFuncSoftPhysics];

	auto defSFuncPortal = luabind::class_<pragma::SFuncPortalComponent,pragma::BaseFuncPortalComponent>("FuncPortalComponent");
	entsMod[defSFuncPortal];

	auto defSWater = luabind::class_<pragma::SWaterComponent,pragma::BaseFuncWaterComponent>("WaterComponent");
	entsMod[defSWater];

	auto defSButton = luabind::class_<pragma::SButtonComponent,pragma::BaseFuncButtonComponent>("ButtonComponent");
	entsMod[defSButton];

	auto defSPlayerSpawn = luabind::class_<pragma::SPlayerSpawnComponent,pragma::BaseEntityComponent>("PlayerSpawnComponent");
	entsMod[defSPlayerSpawn];

	auto defSLogicRelay = luabind::class_<pragma::SLogicRelayComponent,pragma::BaseLogicRelayComponent>("LogicRelayComponent");
	entsMod[defSLogicRelay];

	auto defSBot = luabind::class_<pragma::SBotComponent,pragma::BaseBotComponent>("BotComponent");
	entsMod[defSBot];

	RegisterLuaEntityComponents2(l,entsMod); // Split up to prevent compiler errors
}

void Lua::SoundEmitter::CreateSound(lua_State *l,pragma::SSoundEmitterComponent &hEnt,std::string sndname,uint32_t soundType,bool bTransmit)
{
	auto snd = hEnt.CreateSound(sndname,static_cast<ALSoundType>(soundType),bTransmit);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
void Lua::SoundEmitter::EmitSound(lua_State *l,pragma::SSoundEmitterComponent &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit)
{
	auto snd = hEnt.EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch,bTransmit);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
