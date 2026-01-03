// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

// --template-include-location

module pragma.server;
import :game;

import :entities.components;

void pragma::SGame::InitializeEntityComponents(EntityComponentManager &componentManager)
{
	Game::InitializeEntityComponents(componentManager);
	constexpr auto hideInEditor = ComponentRegInfo::Flags::HideInEditor;
	componentManager.RegisterComponentType<SAIComponent>("ai", {"ai", hideInEditor});
	componentManager.RegisterComponentType<SCharacterComponent>("character", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SColorComponent>("color", {"rendering"});
	componentManager.RegisterComponentType<SSurfaceComponent>("surface", {"world/bounds"});
	componentManager.RegisterComponentType<SScoreComponent>("score", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SFlammableComponent>("flammable", {"gameplay"});
	componentManager.RegisterComponentType<SHealthComponent>("health", {"gameplay"});
	componentManager.RegisterComponentType<SNameComponent>("name", {"name", hideInEditor});
	componentManager.RegisterComponentType<SNetworkedComponent>("networked", {"networking", hideInEditor});
	componentManager.RegisterComponentType<SObservableComponent>("observable", {"gameplay"});
	componentManager.RegisterComponentType<SObserverComponent>("observer", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SPhysicsComponent>("physics", {"physics"});
	componentManager.RegisterComponentType<SPlayerComponent>("player", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SRadiusComponent>("radius", {"world/bounds"});
	componentManager.RegisterComponentType<SFieldAngleComponent>("field_angle", {"world/bounds"});
	componentManager.RegisterComponentType<SRenderComponent>("render", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<SSoundEmitterComponent>("sound_emitter", {"audio"});
	componentManager.RegisterComponentType<SToggleComponent>("toggle", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<STransformComponent>("transform", {"world/transform"});
	componentManager.RegisterComponentType<SVehicleComponent>("vehicle", {"gameplay"});
	componentManager.RegisterComponentType<SWeaponComponent>("weapon", {"gameplay"});
	componentManager.RegisterComponentType<SWheelComponent>("wheel", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SSoundDspComponent>("sound_dsp", {"audio/dsp"});
	componentManager.RegisterComponentType<SSoundDspChorusComponent>("sound_dsp_chorus", {"audio/dsp"});
	componentManager.RegisterComponentType<SSoundDspDistortionComponent>("sound_dsp_distortion", {"audio/dsp"});
	componentManager.RegisterComponentType<SSoundDspEAXReverbComponent>("sound_dsp_eax_reverb", {"audio/dsp"});
	componentManager.RegisterComponentType<SSoundDspEchoComponent>("sound_dsp_echo", {"audio/dsp"});
	componentManager.RegisterComponentType<SSoundDspEqualizerComponent>("sound_dsp_equalizer", {"audio/dsp"});
	componentManager.RegisterComponentType<SSoundDspFlangerComponent>("sound_dsp_flanger", {"audio/dsp"});
	componentManager.RegisterComponentType<SCameraComponent>("camera", {"rendering/camera"});
	componentManager.RegisterComponentType<SDecalComponent>("decal", {"rendering"});
	componentManager.RegisterComponentType<SExplosionComponent>("explosion", {"gameplay/effects"});
	componentManager.RegisterComponentType<SFireComponent>("fire", {"gameplay/effects"});
	componentManager.RegisterComponentType<SFogControllerComponent>("fog_controller", {"rendering/effects"});
	componentManager.RegisterComponentType<SLightComponent>("light", {"rendering/lighting"});
	componentManager.RegisterComponentType<SLightDirectionalComponent>("light_directional", {"rendering/lighting"});
	componentManager.RegisterComponentType<SLightPointComponent>("light_point", {"rendering/lighting"});
	componentManager.RegisterComponentType<SLightSpotComponent>("light_spot", {"rendering/lighting"});
	componentManager.RegisterComponentType<SLightSpotVolComponent>("light_spot_volume", {"rendering/lighting"});
	componentManager.RegisterComponentType<SMicrophoneComponent>("microphone", {"audio"});
	componentManager.RegisterComponentType<SParticleSystemComponent>("particle_system", {"rendering/effects"});
	componentManager.RegisterComponentType<SQuakeComponent>("quake", {"gameplay/effects"});
	componentManager.RegisterComponentType<SSmokeTrailComponent>("smoke_trail", {"rendering/effects"});
	componentManager.RegisterComponentType<SSoundComponent>("sound", {"audio"});
	componentManager.RegisterComponentType<SSoundScapeComponent>("sound_scape", {"audio"});
	componentManager.RegisterComponentType<SSpriteComponent>("sprite", {"rendering/effects"});
	componentManager.RegisterComponentType<SEnvTimescaleComponent>("env_time_scale", {"world"});
	componentManager.RegisterComponentType<SWindComponent>("wind", {"world"});
	componentManager.RegisterComponentType<SFilterClassComponent>("filter_class", {"core", hideInEditor});
	componentManager.RegisterComponentType<SFilterNameComponent>("filter_name", {"core", hideInEditor});
	componentManager.RegisterComponentType<SBrushComponent>("brush", {"world", hideInEditor});
	componentManager.RegisterComponentType<SKinematicComponent>("kinematic", {"physics", hideInEditor});
	componentManager.RegisterComponentType<SFuncPhysicsComponent>("func_physics", {"physics", hideInEditor});
	componentManager.RegisterComponentType<SFuncSoftPhysicsComponent>("func_soft_physics", {"physics", hideInEditor});
	// componentManager.RegisterComponentType<pragma::SFuncPortalComponent>("func_portal", {"physics", hideInEditor});
	componentManager.RegisterComponentType<SButtonComponent>("button", {"physics", hideInEditor});
	componentManager.RegisterComponentType<SPlayerSpawnComponent>("player_spawn", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SLogicRelayComponent>("relay", {"logic", hideInEditor});
	componentManager.RegisterComponentType<SBotComponent>("bot", {"ai", hideInEditor});
	componentManager.RegisterComponentType<SPointConstraintBallSocketComponent>("constraint_ball_socket", {"physics/constraints"});
	componentManager.RegisterComponentType<SPointConstraintConeTwistComponent>("constraint_cone_twist", {"physics/constraints"});
	componentManager.RegisterComponentType<SPointConstraintDoFComponent>("constraint_dof", {"physics/constraints"});
	componentManager.RegisterComponentType<SPointConstraintFixedComponent>("constraint_fixed", {"physics/constraints"});
	componentManager.RegisterComponentType<SPointConstraintHingeComponent>("constraint_hinge", {"physics/constraints"});
	componentManager.RegisterComponentType<SPointConstraintSliderComponent>("constraint_slider", {"physics/constraints"});
	componentManager.RegisterComponentType<SPathNodeComponent>("path_node", {"ai", hideInEditor});
	// componentManager.RegisterComponentType<pragma::SRenderTargetComponent>("render_target", {"rendering"});
	componentManager.RegisterComponentType<SPointTargetComponent>("target", {"world", hideInEditor});
	componentManager.RegisterComponentType<PropComponent>("prop", {"physics"});
	componentManager.RegisterComponentType<SPropDynamicComponent>("prop_dynamic", {"physics"});
	componentManager.RegisterComponentType<SPropPhysicsComponent>("prop_physics", {"physics"});
	componentManager.RegisterComponentType<STriggerGravityComponent>("trigger_gravity", {"gameplay/triggers"});
	componentManager.RegisterComponentType<STriggerHurtComponent>("trigger_hurt", {"gameplay/triggers"});
	componentManager.RegisterComponentType<STriggerPushComponent>("trigger_push", {"gameplay/triggers"});
	componentManager.RegisterComponentType<STriggerRemoveComponent>("trigger_remove", {"gameplay/triggers"});
	componentManager.RegisterComponentType<STriggerTeleportComponent>("trigger_teleport", {"gameplay/triggers"});
	componentManager.RegisterComponentType<STouchComponent>("touch", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SSkyboxComponent>("skybox", {"world"});
	componentManager.RegisterComponentType<SWorldComponent>("world", {"world", hideInEditor});
	componentManager.RegisterComponentType<SFlashlightComponent>("flashlight", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<ecs::SShooterComponent>("shooter", {"gameplay"});
	componentManager.RegisterComponentType<SModelComponent>("model", {"rendering/model"});
	componentManager.RegisterComponentType<SAnimatedComponent>("animated", {"animation"});
	componentManager.RegisterComponentType<SGenericComponent>("entity", {"core", hideInEditor});
	componentManager.RegisterComponentType<SIOComponent>("io", {"core", hideInEditor});
	componentManager.RegisterComponentType<STimeScaleComponent>("time_scale", {"world", hideInEditor});
	componentManager.RegisterComponentType<SInfoLandmarkComponent>("info_landmark", {"world", hideInEditor});
	componentManager.RegisterComponentType<SAttachmentComponent>("attachment", {"world", hideInEditor});
	componentManager.RegisterComponentType<SChildComponent>("child", {"core", hideInEditor});
	componentManager.RegisterComponentType<SOwnableComponent>("ownable", {"core", hideInEditor});
	componentManager.RegisterComponentType<SDebugTextComponent>("debug_text", {"debug"});
	componentManager.RegisterComponentType<SDebugPointComponent>("debug_point", {"debug"});
	componentManager.RegisterComponentType<SDebugLineComponent>("debug_line", {"debug"});
	componentManager.RegisterComponentType<SDebugBoxComponent>("debug_box", {"debug"});
	componentManager.RegisterComponentType<SDebugSphereComponent>("debug_sphere", {"debug"});
	componentManager.RegisterComponentType<SDebugConeComponent>("debug_cone", {"debug"});
	componentManager.RegisterComponentType<SDebugCylinderComponent>("debug_cylinder", {"debug"});
	componentManager.RegisterComponentType<SDebugPlaneComponent>("debug_plane", {"debug"});
	componentManager.RegisterComponentType<SPointAtTargetComponent>("point_at_target", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SGamemodeComponent>("gamemode", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<SGameComponent>("game", {"gameplay", hideInEditor});

	componentManager.RegisterComponentType<SLiquidComponent>("liquid", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<SBuoyancyComponent>("buoyancy", {"physics"});
	componentManager.RegisterComponentType<SLiquidSurfaceComponent>("liquid_surface", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<SLiquidVolumeComponent>("liquid_volume", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<SLiquidControlComponent>("liquid_control", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<SLiquidSurfaceSimulationComponent>("liquid_surface_simulation", {"physics/fluid", hideInEditor});
	// --template-component-register-location
}

pragma::BaseEntityComponent *pragma::SGame::CreateLuaEntityComponent(ecs::BaseEntity &ent, std::string classname) { return Game::CreateLuaEntityComponent<SLuaBaseEntityComponent, LuaCore::SLuaBaseEntityComponentHolder>(ent, classname); }
