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
#include "pragma/entities/components/s_field_angle_component.hpp"
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
#include "pragma/entities/components/s_surface_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_surface_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_volume_component.hpp"
#include "pragma/entities/components/liquid/s_buoyancy_component.hpp"
#include "pragma/entities/func/s_funcbutton.h"
#include "pragma/entities/game_player_spawn.h"
#include "pragma/entities/logic/s_logic_relay.h"
#include "pragma/entities/s_bot.h"
#include "pragma/entities/components/s_shooter_component.hpp"
#include "pragma/entities/components/s_entity_component.hpp"
#include "pragma/entities/components/s_time_scale_component.hpp"
#include "pragma/entities/components/s_gamemode_component.hpp"
#include "pragma/entities/components/s_game_component.hpp"
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
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/netpacket.hpp>

#include <pragma/lua/ostream_operator_alias.hpp>

void SGame::RegisterLuaEntityComponent(luabind::class_<pragma::BaseEntityComponent> &def)
{
	Game::RegisterLuaEntityComponent(def);
	// TODO: Remove this function
}
void RegisterLuaEntityComponents2_sv(lua_State *l, luabind::module_ &entsMod);
namespace Lua::SoundEmitter {
	DLLNETWORK luabind::class_<pragma::BaseSoundEmitterComponent::SoundInfo> RegisterSoundInfo();
};

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEnvSoundDspComponent);
#endif

void SGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);

	auto *l = GetLuaState();
	Lua::register_sv_ai_component(l, entsMod);
	Lua::register_sv_character_component(l, entsMod);
	Lua::register_sv_player_component(l, entsMod);
	Lua::register_sv_vehicle_component(l, entsMod);
	Lua::register_sv_weapon_component(l, entsMod);

	auto defSGamemode = pragma::lua::create_entity_component_class<pragma::SGamemodeComponent, pragma::BaseGamemodeComponent>("GamemodeComponent");
	entsMod[defSGamemode];

	auto defSGame = pragma::lua::create_entity_component_class<pragma::SGameComponent, pragma::BaseGameComponent>("GameComponent");
	entsMod[defSGame];

	auto defSColor = pragma::lua::create_entity_component_class<pragma::SColorComponent, pragma::BaseColorComponent>("ColorComponent");
	entsMod[defSColor];

	auto defSScore = pragma::lua::create_entity_component_class<pragma::SScoreComponent, pragma::BaseScoreComponent>("ScoreComponent");
	entsMod[defSScore];

	auto defSFlammable = pragma::lua::create_entity_component_class<pragma::SFlammableComponent, pragma::BaseFlammableComponent>("FlammableComponent");
	entsMod[defSFlammable];

	auto defSHealth = pragma::lua::create_entity_component_class<pragma::SHealthComponent, pragma::BaseHealthComponent>("HealthComponent");
	entsMod[defSHealth];

	auto defSName = pragma::lua::create_entity_component_class<pragma::SNameComponent, pragma::BaseNameComponent>("NameComponent");
	entsMod[defSName];

	auto defSNetworked = pragma::lua::create_entity_component_class<pragma::SNetworkedComponent, pragma::BaseNetworkedComponent>("NetworkedComponent");
	entsMod[defSNetworked];

	auto defSObservable = pragma::lua::create_entity_component_class<pragma::SObservableComponent, pragma::BaseObservableComponent>("ObservableComponent");
	entsMod[defSObservable];

	auto defSShooter = pragma::lua::create_entity_component_class<pragma::SShooterComponent, pragma::BaseShooterComponent>("ShooterComponent");
	entsMod[defSShooter];

	auto defSPhysics = pragma::lua::create_entity_component_class<pragma::SPhysicsComponent, pragma::BasePhysicsComponent>("PhysicsComponent");
	entsMod[defSPhysics];

	auto defSRadius = pragma::lua::create_entity_component_class<pragma::SRadiusComponent, pragma::BaseRadiusComponent>("RadiusComponent");
	entsMod[defSRadius];

	auto defSFieldAngle = pragma::lua::create_entity_component_class<pragma::SFieldAngleComponent, pragma::BaseFieldAngleComponent>("FieldAngleComponent");
	entsMod[defSFieldAngle];

	auto defSRender = pragma::lua::create_entity_component_class<pragma::SRenderComponent, pragma::BaseRenderComponent>("RenderComponent");
	entsMod[defSRender];

	auto defSSoundEmitter = pragma::lua::create_entity_component_class<pragma::SSoundEmitterComponent, pragma::BaseSoundEmitterComponent>("SoundEmitterComponent");
	defSSoundEmitter.scope[Lua::SoundEmitter::RegisterSoundInfo()];
	entsMod[defSSoundEmitter];

	auto defSToggle = pragma::lua::create_entity_component_class<pragma::SToggleComponent, pragma::BaseToggleComponent>("ToggleComponent");
	entsMod[defSToggle];

	auto defSTransform = pragma::lua::create_entity_component_class<pragma::STransformComponent, pragma::BaseTransformComponent>("TransformComponent");
	defSTransform.add_static_constant("EVENT_ON_POSE_CHANGED", pragma::STransformComponent::EVENT_ON_POSE_CHANGED);
	defSTransform.add_static_constant("EVENT_ON_TELEPORT", pragma::STransformComponent::EVENT_ON_TELEPORT);
	defSTransform.def("Teleport", &pragma::STransformComponent::Teleport);
	entsMod[defSTransform];

	auto defSWheel = pragma::lua::create_entity_component_class<pragma::SWheelComponent, pragma::BaseWheelComponent>("WheelComponent");
	entsMod[defSWheel];

	auto defSSoundDsp = pragma::lua::create_entity_component_class<pragma::SSoundDspComponent, pragma::BaseEnvSoundDspComponent>("SoundDspComponent");
	entsMod[defSSoundDsp];

	auto defSSoundDspChorus = pragma::lua::create_entity_component_class<pragma::SSoundDspChorusComponent, pragma::BaseEnvSoundDspComponent>("SoundDspChorusComponent");
	entsMod[defSSoundDspChorus];

	auto defSSoundDspDistortion = pragma::lua::create_entity_component_class<pragma::SSoundDspDistortionComponent, pragma::BaseEnvSoundDspComponent>("SoundDspDistortionComponent");
	entsMod[defSSoundDspDistortion];

	auto defSSoundDspEAXReverb = pragma::lua::create_entity_component_class<pragma::SSoundDspEAXReverbComponent, pragma::BaseEnvSoundDspComponent>("SoundDspEAXReverbComponent");
	entsMod[defSSoundDspEAXReverb];

	auto defSSoundDspEcho = pragma::lua::create_entity_component_class<pragma::SSoundDspEchoComponent, pragma::BaseEnvSoundDspComponent>("SoundDspEchoComponent");
	entsMod[defSSoundDspEcho];

	auto defSSoundDspEqualizer = pragma::lua::create_entity_component_class<pragma::SSoundDspEqualizerComponent, pragma::BaseEnvSoundDspComponent>("SoundDspEqualizerComponent");
	entsMod[defSSoundDspEqualizer];

	auto defSSoundDspFlanger = pragma::lua::create_entity_component_class<pragma::SSoundDspFlangerComponent, pragma::BaseEnvSoundDspComponent>("SoundDspFlangerComponent");
	entsMod[defSSoundDspFlanger];

	auto defSCamera = pragma::lua::create_entity_component_class<pragma::SCameraComponent, pragma::BaseEnvCameraComponent>("CameraComponent");
	entsMod[defSCamera];

	auto defSDecal = pragma::lua::create_entity_component_class<pragma::SDecalComponent, pragma::BaseEnvDecalComponent>("DecalComponent");
	entsMod[defSDecal];

	auto defSExplosion = pragma::lua::create_entity_component_class<pragma::SExplosionComponent, pragma::BaseEnvExplosionComponent>("ExplosionComponent");
	entsMod[defSExplosion];

	auto defSFire = pragma::lua::create_entity_component_class<pragma::SFireComponent, pragma::BaseEnvFireComponent>("FireComponent");
	entsMod[defSFire];

	auto defSFogController = pragma::lua::create_entity_component_class<pragma::SFogControllerComponent, pragma::BaseEnvFogControllerComponent>("FogControllerComponent");
	entsMod[defSFogController];

	auto defSLight = pragma::lua::create_entity_component_class<pragma::SLightComponent, pragma::BaseEnvLightComponent>("LightComponent");
	entsMod[defSLight];

	auto defSLightDirectional = pragma::lua::create_entity_component_class<pragma::SLightDirectionalComponent, pragma::BaseEnvLightDirectionalComponent>("LightDirectionalComponent");
	entsMod[defSLightDirectional];

	auto defSLightPoint = pragma::lua::create_entity_component_class<pragma::SLightPointComponent, pragma::BaseEnvLightPointComponent>("LightPointComponent");
	entsMod[defSLightPoint];

	auto defSLightSpot = pragma::lua::create_entity_component_class<pragma::SLightSpotComponent, pragma::BaseEnvLightSpotComponent>("LightSpotComponent");
	entsMod[defSLightSpot];

	auto defSLightSpotVol = pragma::lua::create_entity_component_class<pragma::SLightSpotVolComponent, pragma::BaseEnvLightSpotVolComponent>("LightSpotVolComponent");
	entsMod[defSLightSpotVol];

	auto defSMicrophone = pragma::lua::create_entity_component_class<pragma::SMicrophoneComponent, pragma::BaseEnvMicrophoneComponent>("MicrophoneComponent");
	entsMod[defSMicrophone];

	auto defSParticleSystem = pragma::lua::create_entity_component_class<pragma::SParticleSystemComponent, pragma::BaseEnvParticleSystemComponent>("ParticleSystemComponent");
	entsMod[defSParticleSystem];

	auto defSQuake = pragma::lua::create_entity_component_class<pragma::SQuakeComponent, pragma::BaseEnvQuakeComponent>("QuakeComponent");
	entsMod[defSQuake];

	auto defSSmokeTrail = pragma::lua::create_entity_component_class<pragma::SSmokeTrailComponent, pragma::BaseEnvSmokeTrailComponent>("SmokeTrailComponent");
	entsMod[defSSmokeTrail];

	auto defSSound = pragma::lua::create_entity_component_class<pragma::SSoundComponent, pragma::BaseEnvSoundComponent>("SoundComponent");
	entsMod[defSSound];

	auto defSSoundScape = pragma::lua::create_entity_component_class<pragma::SSoundScapeComponent, pragma::BaseEnvSoundScapeComponent>("SoundScapeComponent");
	entsMod[defSSoundScape];

	auto defSSprite = pragma::lua::create_entity_component_class<pragma::SSpriteComponent, pragma::BaseEnvSpriteComponent>("SpriteComponent");
	entsMod[defSSprite];

	auto defSTimescale = pragma::lua::create_entity_component_class<pragma::SEnvTimescaleComponent, pragma::BaseEnvTimescaleComponent>("EnvTimeScaleComponent");
	entsMod[defSTimescale];

	auto defSWind = pragma::lua::create_entity_component_class<pragma::SWindComponent, pragma::BaseEnvWindComponent>("WindComponent");
	entsMod[defSWind];

	auto defSFilterClass = pragma::lua::create_entity_component_class<pragma::SFilterClassComponent, pragma::BaseFilterClassComponent>("FilterClassComponent");
	entsMod[defSFilterClass];

	auto defSFilterName = pragma::lua::create_entity_component_class<pragma::SFilterNameComponent, pragma::BaseFilterNameComponent>("FilterNameComponent");
	entsMod[defSFilterName];

	auto defSBrush = pragma::lua::create_entity_component_class<pragma::SBrushComponent, pragma::BaseFuncBrushComponent>("BrushComponent");
	entsMod[defSBrush];

	auto defSKinematic = pragma::lua::create_entity_component_class<pragma::SKinematicComponent, pragma::BaseFuncKinematicComponent>("KinematicComponent");
	entsMod[defSKinematic];

	auto defSFuncPhysics = pragma::lua::create_entity_component_class<pragma::SFuncPhysicsComponent, pragma::BaseFuncPhysicsComponent>("FuncPhysicsComponent");
	entsMod[defSFuncPhysics];

	auto defSFuncSoftPhysics = pragma::lua::create_entity_component_class<pragma::SFuncSoftPhysicsComponent, pragma::BaseFuncSoftPhysicsComponent>("FuncSoftPhysicsComponent");
	entsMod[defSFuncSoftPhysics];

	auto defSSurface = pragma::lua::create_entity_component_class<pragma::SSurfaceComponent, pragma::BaseSurfaceComponent>("SurfaceComponent");
	entsMod[defSSurface];

	// auto defSFuncPortal = pragma::lua::create_entity_component_class<pragma::SFuncPortalComponent,pragma::BaseFuncPortalComponent>("FuncPortalComponent");
	// entsMod[defSFuncPortal];

	auto defSLiquidSurf = pragma::lua::create_entity_component_class<pragma::SLiquidSurfaceComponent, pragma::BaseLiquidSurfaceComponent>("LiquidSurfaceComponent");
	entsMod[defSLiquidSurf];

	auto defSLiquidVol = pragma::lua::create_entity_component_class<pragma::SLiquidVolumeComponent, pragma::BaseLiquidVolumeComponent>("LiquidVolumeComponent");
	entsMod[defSLiquidVol];

	auto defSBuoyancy = pragma::lua::create_entity_component_class<pragma::SBuoyancyComponent, pragma::BaseBuoyancyComponent>("BuoyancyComponent");
	entsMod[defSBuoyancy];

	auto defSWater = pragma::lua::create_entity_component_class<pragma::SLiquidComponent, pragma::BaseFuncLiquidComponent>("LiquidComponent");
	entsMod[defSWater];

	auto defSButton = pragma::lua::create_entity_component_class<pragma::SButtonComponent, pragma::BaseFuncButtonComponent>("ButtonComponent");
	entsMod[defSButton];

	auto defSPlayerSpawn = pragma::lua::create_entity_component_class<pragma::SPlayerSpawnComponent, pragma::BaseEntityComponent>("PlayerSpawnComponent");
	entsMod[defSPlayerSpawn];

	auto defSLogicRelay = pragma::lua::create_entity_component_class<pragma::SLogicRelayComponent, pragma::BaseLogicRelayComponent>("LogicRelayComponent");
	entsMod[defSLogicRelay];

	auto defSBot = pragma::lua::create_entity_component_class<pragma::SBotComponent, pragma::BaseBotComponent>("BotComponent");
	entsMod[defSBot];

	RegisterLuaEntityComponents2_sv(l, entsMod); // Split up to prevent compiler errors
}
