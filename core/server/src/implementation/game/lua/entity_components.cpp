// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"


module pragma.server.game;

import pragma.entities.components;
import pragma.server.entities;
import pragma.server.entities.components;

void SGame::RegisterLuaEntityComponent(luabind::class_<pragma::BaseEntityComponent> &def)
{
	Game::RegisterLuaEntityComponent(def);
	// TODO: Remove this function
}
void RegisterLuaEntityComponents2_sv(lua_State *l, luabind::module_ &entsMod);

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

	auto defSObserver = pragma::lua::create_entity_component_class<pragma::SObserverComponent, pragma::BaseObserverComponent>("ObserverComponent");
	entsMod[defSObserver];

	auto defSShooter = pragma::lua::create_entity_component_class<pragma::ecs::SShooterComponent, pragma::ecs::BaseShooterComponent>("ShooterComponent");
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
