// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.server;
import :game;

import :entities;
import :entities.components;
import pragma.shared;

void RegisterLuaEntityComponents2_sv(lua::State *l, luabind::module_ &entsMod);

void pragma::SGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);

	auto defSGamemode = pragma::LuaCore::create_entity_component_class<SGamemodeComponent, BaseGamemodeComponent>("GamemodeComponent");
	entsMod[defSGamemode];

	auto defSGame = pragma::LuaCore::create_entity_component_class<SGameComponent, BaseGameComponent>("GameComponent");
	entsMod[defSGame];

	auto defSColor = pragma::LuaCore::create_entity_component_class<SColorComponent, BaseColorComponent>("ColorComponent");
	entsMod[defSColor];

	auto defSScore = pragma::LuaCore::create_entity_component_class<SScoreComponent, BaseScoreComponent>("ScoreComponent");
	entsMod[defSScore];

	auto defSFlammable = pragma::LuaCore::create_entity_component_class<SFlammableComponent, BaseFlammableComponent>("FlammableComponent");
	entsMod[defSFlammable];

	auto defSHealth = pragma::LuaCore::create_entity_component_class<SHealthComponent, BaseHealthComponent>("HealthComponent");
	entsMod[defSHealth];

	auto defSName = pragma::LuaCore::create_entity_component_class<SNameComponent, BaseNameComponent>("NameComponent");
	entsMod[defSName];

	auto defSNetworked = pragma::LuaCore::create_entity_component_class<SNetworkedComponent, BaseNetworkedComponent>("NetworkedComponent");
	entsMod[defSNetworked];

	auto defSObservable = pragma::LuaCore::create_entity_component_class<SObservableComponent, BaseObservableComponent>("ObservableComponent");
	entsMod[defSObservable];

	auto defSObserver = pragma::LuaCore::create_entity_component_class<SObserverComponent, BaseObserverComponent>("ObserverComponent");
	entsMod[defSObserver];

	auto defSShooter = pragma::LuaCore::create_entity_component_class<ecs::SShooterComponent, ecs::BaseShooterComponent>("ShooterComponent");
	entsMod[defSShooter];

	auto defSPhysics = pragma::LuaCore::create_entity_component_class<SPhysicsComponent, BasePhysicsComponent>("PhysicsComponent");
	entsMod[defSPhysics];

	auto defSRadius = pragma::LuaCore::create_entity_component_class<SRadiusComponent, BaseRadiusComponent>("RadiusComponent");
	entsMod[defSRadius];

	auto defSFieldAngle = pragma::LuaCore::create_entity_component_class<SFieldAngleComponent, BaseFieldAngleComponent>("FieldAngleComponent");
	entsMod[defSFieldAngle];

	auto defSRender = pragma::LuaCore::create_entity_component_class<SRenderComponent, BaseRenderComponent>("RenderComponent");
	entsMod[defSRender];

	auto defSSoundEmitter = pragma::LuaCore::create_entity_component_class<SSoundEmitterComponent, BaseSoundEmitterComponent>("SoundEmitterComponent");
	defSSoundEmitter.scope[Lua::SoundEmitter::RegisterSoundInfo()];
	entsMod[defSSoundEmitter];

	auto defSToggle = pragma::LuaCore::create_entity_component_class<SToggleComponent, BaseToggleComponent>("ToggleComponent");
	entsMod[defSToggle];

	auto defSTransform = pragma::LuaCore::create_entity_component_class<STransformComponent, BaseTransformComponent>("TransformComponent");
	defSTransform.add_static_constant("EVENT_ON_POSE_CHANGED", sTransformComponent::EVENT_ON_POSE_CHANGED);
	defSTransform.add_static_constant("EVENT_ON_TELEPORT", sTransformComponent::EVENT_ON_TELEPORT);
	defSTransform.def("Teleport", &STransformComponent::Teleport);
	entsMod[defSTransform];

	auto defSWheel = pragma::LuaCore::create_entity_component_class<SWheelComponent, BaseWheelComponent>("WheelComponent");
	entsMod[defSWheel];

	auto defSSoundDsp = pragma::LuaCore::create_entity_component_class<SSoundDspComponent, BaseEnvSoundDspComponent>("SoundDspComponent");
	entsMod[defSSoundDsp];

	auto defSSoundDspChorus = pragma::LuaCore::create_entity_component_class<SSoundDspChorusComponent, BaseEnvSoundDspComponent>("SoundDspChorusComponent");
	entsMod[defSSoundDspChorus];

	auto defSSoundDspDistortion = pragma::LuaCore::create_entity_component_class<SSoundDspDistortionComponent, BaseEnvSoundDspComponent>("SoundDspDistortionComponent");
	entsMod[defSSoundDspDistortion];

	auto defSSoundDspEAXReverb = pragma::LuaCore::create_entity_component_class<SSoundDspEAXReverbComponent, BaseEnvSoundDspComponent>("SoundDspEAXReverbComponent");
	entsMod[defSSoundDspEAXReverb];

	auto defSSoundDspEcho = pragma::LuaCore::create_entity_component_class<SSoundDspEchoComponent, BaseEnvSoundDspComponent>("SoundDspEchoComponent");
	entsMod[defSSoundDspEcho];

	auto defSSoundDspEqualizer = pragma::LuaCore::create_entity_component_class<SSoundDspEqualizerComponent, BaseEnvSoundDspComponent>("SoundDspEqualizerComponent");
	entsMod[defSSoundDspEqualizer];

	auto defSSoundDspFlanger = pragma::LuaCore::create_entity_component_class<SSoundDspFlangerComponent, BaseEnvSoundDspComponent>("SoundDspFlangerComponent");
	entsMod[defSSoundDspFlanger];

	auto defSCamera = pragma::LuaCore::create_entity_component_class<SCameraComponent, BaseEnvCameraComponent>("CameraComponent");
	entsMod[defSCamera];

	auto defSDecal = pragma::LuaCore::create_entity_component_class<SDecalComponent, BaseEnvDecalComponent>("DecalComponent");
	entsMod[defSDecal];

	auto defSExplosion = pragma::LuaCore::create_entity_component_class<SExplosionComponent, BaseEnvExplosionComponent>("ExplosionComponent");
	entsMod[defSExplosion];

	auto defSFire = pragma::LuaCore::create_entity_component_class<SFireComponent, BaseEnvFireComponent>("FireComponent");
	entsMod[defSFire];

	auto defSFogController = pragma::LuaCore::create_entity_component_class<SFogControllerComponent, BaseEnvFogControllerComponent>("FogControllerComponent");
	entsMod[defSFogController];

	auto defSLight = pragma::LuaCore::create_entity_component_class<SLightComponent, BaseEnvLightComponent>("LightComponent");
	entsMod[defSLight];

	auto defSLightDirectional = pragma::LuaCore::create_entity_component_class<SLightDirectionalComponent, BaseEnvLightDirectionalComponent>("LightDirectionalComponent");
	entsMod[defSLightDirectional];

	auto defSLightPoint = pragma::LuaCore::create_entity_component_class<SLightPointComponent, BaseEnvLightPointComponent>("LightPointComponent");
	entsMod[defSLightPoint];

	auto defSLightSpot = pragma::LuaCore::create_entity_component_class<SLightSpotComponent, BaseEnvLightSpotComponent>("LightSpotComponent");
	entsMod[defSLightSpot];

	auto defSLightSpotVol = pragma::LuaCore::create_entity_component_class<SLightSpotVolComponent, BaseEnvLightSpotVolComponent>("LightSpotVolComponent");
	entsMod[defSLightSpotVol];

	auto defSMicrophone = pragma::LuaCore::create_entity_component_class<SMicrophoneComponent, BaseEnvMicrophoneComponent>("MicrophoneComponent");
	entsMod[defSMicrophone];

	auto defSParticleSystem = pragma::LuaCore::create_entity_component_class<SParticleSystemComponent, BaseEnvParticleSystemComponent>("ParticleSystemComponent");
	entsMod[defSParticleSystem];

	auto defSQuake = pragma::LuaCore::create_entity_component_class<SQuakeComponent, BaseEnvQuakeComponent>("QuakeComponent");
	entsMod[defSQuake];

	auto defSSmokeTrail = pragma::LuaCore::create_entity_component_class<SSmokeTrailComponent, BaseEnvSmokeTrailComponent>("SmokeTrailComponent");
	entsMod[defSSmokeTrail];

	auto defSSound = pragma::LuaCore::create_entity_component_class<SSoundComponent, BaseEnvSoundComponent>("SoundComponent");
	entsMod[defSSound];

	auto defSSoundScape = pragma::LuaCore::create_entity_component_class<SSoundScapeComponent, BaseEnvSoundScapeComponent>("SoundScapeComponent");
	entsMod[defSSoundScape];

	auto defSSprite = pragma::LuaCore::create_entity_component_class<SSpriteComponent, BaseEnvSpriteComponent>("SpriteComponent");
	entsMod[defSSprite];

	auto defSTimescale = pragma::LuaCore::create_entity_component_class<SEnvTimescaleComponent, BaseEnvTimescaleComponent>("EnvTimeScaleComponent");
	entsMod[defSTimescale];

	auto defSWind = pragma::LuaCore::create_entity_component_class<SWindComponent, BaseEnvWindComponent>("WindComponent");
	entsMod[defSWind];

	auto defSFilterClass = pragma::LuaCore::create_entity_component_class<SFilterClassComponent, BaseFilterClassComponent>("FilterClassComponent");
	entsMod[defSFilterClass];

	auto defSFilterName = pragma::LuaCore::create_entity_component_class<SFilterNameComponent, BaseFilterNameComponent>("FilterNameComponent");
	entsMod[defSFilterName];

	auto defSBrush = pragma::LuaCore::create_entity_component_class<SBrushComponent, BaseFuncBrushComponent>("BrushComponent");
	entsMod[defSBrush];

	auto defSKinematic = pragma::LuaCore::create_entity_component_class<SKinematicComponent, BaseFuncKinematicComponent>("KinematicComponent");
	entsMod[defSKinematic];

	auto defSFuncPhysics = pragma::LuaCore::create_entity_component_class<SFuncPhysicsComponent, BaseFuncPhysicsComponent>("FuncPhysicsComponent");
	entsMod[defSFuncPhysics];

	auto defSFuncSoftPhysics = pragma::LuaCore::create_entity_component_class<SFuncSoftPhysicsComponent, BaseFuncSoftPhysicsComponent>("FuncSoftPhysicsComponent");
	entsMod[defSFuncSoftPhysics];

	auto defSSurface = pragma::LuaCore::create_entity_component_class<SSurfaceComponent, BaseSurfaceComponent>("SurfaceComponent");
	entsMod[defSSurface];

	// auto defSFuncPortal = pragma::LuaCore::create_entity_component_class<pragma::SFuncPortalComponent,pragma::BaseFuncPortalComponent>("FuncPortalComponent");
	// entsMod[defSFuncPortal];

	auto defSLiquidSurf = pragma::LuaCore::create_entity_component_class<SLiquidSurfaceComponent, BaseLiquidSurfaceComponent>("LiquidSurfaceComponent");
	entsMod[defSLiquidSurf];

	auto defSLiquidVol = pragma::LuaCore::create_entity_component_class<SLiquidVolumeComponent, BaseLiquidVolumeComponent>("LiquidVolumeComponent");
	entsMod[defSLiquidVol];

	auto defSBuoyancy = pragma::LuaCore::create_entity_component_class<SBuoyancyComponent, BaseBuoyancyComponent>("BuoyancyComponent");
	entsMod[defSBuoyancy];

	auto defSWater = pragma::LuaCore::create_entity_component_class<SLiquidComponent, BaseFuncLiquidComponent>("LiquidComponent");
	entsMod[defSWater];

	auto defSButton = pragma::LuaCore::create_entity_component_class<SButtonComponent, BaseFuncButtonComponent>("ButtonComponent");
	entsMod[defSButton];

	auto defSPlayerSpawn = pragma::LuaCore::create_entity_component_class<SPlayerSpawnComponent, BaseEntityComponent>("PlayerSpawnComponent");
	entsMod[defSPlayerSpawn];

	auto defSLogicRelay = pragma::LuaCore::create_entity_component_class<SLogicRelayComponent, BaseLogicRelayComponent>("LogicRelayComponent");
	entsMod[defSLogicRelay];

	auto defSBot = pragma::LuaCore::create_entity_component_class<SBotComponent, BaseBotComponent>("BotComponent");
	entsMod[defSBot];

	auto *l = GetLuaState();
	RegisterLuaEntityComponents2_sv(l, entsMod); // Split up to prevent compiler errors
}
