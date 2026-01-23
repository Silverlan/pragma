// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

// --template-include-location

module pragma.client;

import :game;
import :entities.components;

void pragma::CGame::InitializeEntityComponents(EntityComponentManager &componentManager)
{
	Game::InitializeEntityComponents(componentManager);
	ecs::CBaseEntity::RegisterEvents(componentManager);
	constexpr auto hideInEditor = ComponentRegInfo::Flags::HideInEditor;
	componentManager.RegisterComponentType<CAIComponent>("ai", {"ai", hideInEditor});
	componentManager.RegisterComponentType<CCharacterComponent>("character", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CColorComponent>("color", {"rendering"});
	componentManager.RegisterComponentType<CSurfaceComponent>("surface", {"world/bounds"});
	componentManager.RegisterComponentType<CScoreComponent>("score", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CFlammableComponent>("flammable", {"gameplay"});
	componentManager.RegisterComponentType<CHealthComponent>("health", {"gameplay"});
	componentManager.RegisterComponentType<CNameComponent>("name", {"core", hideInEditor});
	componentManager.RegisterComponentType<CNetworkedComponent>("networked", {"networking", hideInEditor});
	componentManager.RegisterComponentType<CObservableComponent>("observable", {"gameplay"});
	componentManager.RegisterComponentType<CObserverComponent>("observer", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CPhysicsComponent>("physics", {"physics"});
	componentManager.RegisterComponentType<CPlayerComponent>("player", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CRadiusComponent>("radius", {"world/bounds"});
	componentManager.RegisterComponentType<CFieldAngleComponent>("field_angle", {"world/bounds"});
	componentManager.RegisterComponentType<CRenderComponent>("render", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CSoundEmitterComponent>("sound_emitter", {"audio"});
	componentManager.RegisterComponentType<CToggleComponent>("toggle", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CTransformComponent>("transform", {"world/transform"});
	componentManager.RegisterComponentType<CVehicleComponent>("vehicle", {"gameplay"});
	componentManager.RegisterComponentType<CWeaponComponent>("weapon", {"gameplay"});
	componentManager.RegisterComponentType<CWheelComponent>("wheel", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CSoundDspComponent>("sound_dsp", {"audio/dsp"});
	componentManager.RegisterComponentType<CSoundDspChorusComponent>("sound_dsp_chorus", {"audio/dsp"});
	componentManager.RegisterComponentType<CSoundDspDistortionComponent>("sound_dsp_distortion", {"audio/dsp"});
	componentManager.RegisterComponentType<CSoundDspEAXReverbComponent>("sound_dsp_eax_reverb", {"audio/dsp"});
	componentManager.RegisterComponentType<CSoundDspEchoComponent>("sound_dsp_echo", {"audio/dsp"});
	componentManager.RegisterComponentType<CSoundDspEqualizerComponent>("sound_dsp_equalizer", {"audio/dsp"});
	componentManager.RegisterComponentType<CSoundDspFlangerComponent>("sound_dsp_flanger", {"audio/dsp"});
	componentManager.RegisterComponentType<CCameraComponent>("camera", {"rendering/camera"});
	componentManager.RegisterComponentType<CDecalComponent>("decal", {"rendering"});
	componentManager.RegisterComponentType<CExplosionComponent>("explosion", {"gameplay/effects"});
	componentManager.RegisterComponentType<CFireComponent>("fire", {"gameplay/effects"});
	componentManager.RegisterComponentType<CFogControllerComponent>("fog_controller", {"rendering/effects"});
	componentManager.RegisterComponentType<CLightComponent>("light", {"rendering/lighting"});
	componentManager.RegisterComponentType<CLightDirectionalComponent>("light_directional", {"rendering/lighting"});
	componentManager.RegisterComponentType<CLightPointComponent>("light_point", {"rendering/lighting"});
	componentManager.RegisterComponentType<CLightSpotComponent>("light_spot", {"rendering/lighting"});
	componentManager.RegisterComponentType<CLightSpotVolComponent>("light_spot_volume", {"rendering/lighting"});
	componentManager.RegisterComponentType<CMicrophoneComponent>("microphone", {"audio"});
	componentManager.RegisterComponentType<ecs::CParticleSystemComponent>("particle_system", {"rendering/effects"});
	componentManager.RegisterComponentType<CQuakeComponent>("quake", {"gameplay/effects"});
	componentManager.RegisterComponentType<CSmokeTrailComponent>("smoke_trail", {"rendering/effects"});
	componentManager.RegisterComponentType<CSoundComponent>("sound", {"audio"});
	componentManager.RegisterComponentType<CSoundScapeComponent>("sound_scape", {"audio"});
	componentManager.RegisterComponentType<CSpriteComponent>("sprite", {"rendering/effects"});
	componentManager.RegisterComponentType<CEnvTimescaleComponent>("env_time_scale", {"world"});
	componentManager.RegisterComponentType<CWindComponent>("wind", {"world"});
	componentManager.RegisterComponentType<CBrushComponent>("brush", {"world", hideInEditor});
	componentManager.RegisterComponentType<CKinematicComponent>("kinematic", {"physics", hideInEditor});
	componentManager.RegisterComponentType<CFuncPhysicsComponent>("func_physics", {"physics", hideInEditor});
	// componentManager.RegisterComponentType<pragma::CFuncPortalComponent>("func_portal", {"physics", hideInEditor});
	componentManager.RegisterComponentType<CButtonComponent>("button", {"physics", hideInEditor});
	componentManager.RegisterComponentType<CBotComponent>("bot", {"ai", hideInEditor});
	componentManager.RegisterComponentType<CPointConstraintBallSocketComponent>("constraint_ball_socket", {"physics/constraints"});
	componentManager.RegisterComponentType<CPointConstraintConeTwistComponent>("constraint_cone_twist", {"physics/constraints"});
	componentManager.RegisterComponentType<CPointConstraintDoFComponent>("constraint_dof", {"physics/constraints"});
	componentManager.RegisterComponentType<CPointConstraintFixedComponent>("constraint_fixed", {"physics/constraints"});
	componentManager.RegisterComponentType<CPointConstraintHingeComponent>("constraint_hinge", {"physics/constraints"});
	componentManager.RegisterComponentType<CPointConstraintSliderComponent>("constraint_slider", {"physics/constraints"});
	componentManager.RegisterComponentType<CPointTargetComponent>("target", {"world", hideInEditor});
	componentManager.RegisterComponentType<CPropComponent>("prop", {"physics"});
	componentManager.RegisterComponentType<CPropDynamicComponent>("prop_dynamic", {"physics"});
	componentManager.RegisterComponentType<CPropPhysicsComponent>("prop_physics", {"physics"});
	componentManager.RegisterComponentType<CTouchComponent>("touch", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CSkyboxComponent>("skybox", {"world"});
	componentManager.RegisterComponentType<CWorldComponent>("world", {"world", hideInEditor});
	componentManager.RegisterComponentType<ecs::CShooterComponent>("shooter", {"gameplay"});
	componentManager.RegisterComponentType<CModelComponent>("model", {"rendering/model"});
	componentManager.RegisterComponentType<CMaterialOverrideComponent>("material_override", {"rendering/model"});
	componentManager.RegisterComponentType<CMaterialPropertyOverrideComponent>("material_property_override", {"rendering/model"});
	componentManager.RegisterComponentType<CAnimatedComponent>("animated", {"animation"});
	componentManager.RegisterComponentType<CGenericComponent>("entity", {"core", hideInEditor});
	componentManager.RegisterComponentType<CIOComponent>("io", {"core", hideInEditor});
	componentManager.RegisterComponentType<CTimeScaleComponent>("time_scale", {"world", hideInEditor});
	componentManager.RegisterComponentType<CAttachmentComponent>("attachment", {"world", hideInEditor});
	componentManager.RegisterComponentType<CReflectionProbeComponent>("reflection_probe", {"rendering"});
	componentManager.RegisterComponentType<CPBRConverterComponent>("pbr_converter", {"util", hideInEditor});
	componentManager.RegisterComponentType<CShadowComponent>("shadow_map", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CShadowCSMComponent>("csm", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CShadowManagerComponent>("shadow_manager", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<COcclusionCullerComponent>("occlusion_culler", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CSceneComponent>("scene", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CGamemodeComponent>("gamemode", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CGameComponent>("game", {"gameplay", hideInEditor});

	componentManager.RegisterComponentType<CLiquidComponent>("liquid", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<CBuoyancyComponent>("buoyancy", {"physics"});
	componentManager.RegisterComponentType<CLiquidSurfaceComponent>("liquid_surface", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<CLiquidVolumeComponent>("liquid_volume", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<CWaterSurfaceComponent>("water_surface", {"physics/fluid", hideInEditor});

	componentManager.RegisterComponentType<CEnvSoundProbeComponent>("sound_probe", {"audio"});
	componentManager.RegisterComponentType<CWeatherComponent>("weather", {"rendering"});
	componentManager.RegisterComponentType<CFuncSoftPhysicsComponent>("func_soft_physics", {"physics", hideInEditor});
	componentManager.RegisterComponentType<CFlashlightComponent>("flashlight", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CListenerComponent>("listener", {"audio", hideInEditor});
	componentManager.RegisterComponentType<CViewBodyComponent>("view_body", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CViewModelComponent>("view_model", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CFlexComponent>("flex", {"animation"});
	componentManager.RegisterComponentType<CSoftBodyComponent>("softbody", {"physics"});
	componentManager.RegisterComponentType<CVertexAnimatedComponent>("vertex_animated", {"animation"});
	componentManager.RegisterComponentType<CSkyCameraComponent>("sky_camera", {"rendering/camera"});
	componentManager.RegisterComponentType<COwnableComponent>("ownable", {"core", hideInEditor});
	componentManager.RegisterComponentType<CDebugTextComponent>("debug_text", {"debug"});
	componentManager.RegisterComponentType<CDebugPointComponent>("debug_point", {"debug"});
	componentManager.RegisterComponentType<CDebugLineComponent>("debug_line", {"debug"});
	componentManager.RegisterComponentType<CDebugBoxComponent>("debug_box", {"debug"});
	componentManager.RegisterComponentType<CDebugSphereComponent>("debug_sphere", {"debug"});
	componentManager.RegisterComponentType<CDebugConeComponent>("debug_cone", {"debug"});
	componentManager.RegisterComponentType<CDebugCylinderComponent>("debug_cylinder", {"debug"});
	componentManager.RegisterComponentType<CDebugPlaneComponent>("debug_plane", {"debug"});
	componentManager.RegisterComponentType<CBSPLeafComponent>("bsp_leaf", {"world", hideInEditor});
	componentManager.RegisterComponentType<CPointAtTargetComponent>("point_at_target", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<CRaytracingComponent>("raytracing", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CEyeComponent>("eye", {"rendering/model"});
	componentManager.RegisterComponentType<CBvhComponent>("bvh", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<CAnimatedBvhComponent>("animated_bvh", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<CStaticBvhCacheComponent>("static_bvh_cache", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<CStaticBvhUserComponent>("static_bvh_user", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<COpticalCameraComponent>("optical_camera", {"rendering/camera"});
	componentManager.RegisterComponentType<CHitboxBvhComponent>("hitbox_bvh", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<CChildComponent>("child", {"core", hideInEditor});
	componentManager.RegisterComponentType<CInputComponent>("input", {"core", hideInEditor});

	componentManager.RegisterComponentType<CRendererPpFogComponent>("renderer_pp_fog", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<CRendererPpDoFComponent>("renderer_pp_dof", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<CRendererPpBloomComponent>("renderer_pp_bloom", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<CRendererPpToneMappingComponent>("renderer_pp_tone_mapping", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<CRendererPpFxaaComponent>("renderer_pp_fxaa", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<CRendererPpMotionBlurComponent>("renderer_pp_motion_blur", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<CRendererPpVolumetricComponent>("renderer_pp_volumetric", {"rendering/post_processing", hideInEditor});

	componentManager.RegisterComponentType<CMotionBlurDataComponent>("motion_blur_data", {"rendering/camera"});
	componentManager.RegisterComponentType<CBSPComponent>("bsp", {"physics", hideInEditor});
	componentManager.RegisterComponentType<CLightMapComponent>("light_map", {"rendering/lighting", hideInEditor});
	componentManager.RegisterComponentType<CLightMapReceiverComponent>("light_map_receiver", {"rendering/lighting", hideInEditor});
	componentManager.RegisterComponentType<CLightMapDataCacheComponent>("light_map_data_cache", {"rendering/lighting", hideInEditor});
	componentManager.RegisterComponentType<CRendererComponent>("renderer", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CRasterizationRendererComponent>("rasterization_renderer", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CRaytracingRendererComponent>("raytracing_renderer", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<CLiquidControlComponent>("liquid_control", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<CLiquidSurfaceSimulationComponent>("liquid_surface_simulation", {"physics/fluid", hideInEditor});

	componentManager.RegisterComponentType<CGlobalShaderInputComponent>("global_shader_input", {"rendering"});

	componentManager.RegisterComponentType<CDebugHitboxComponent>("debug_hitbox", {"debug"});
	// --template-component-register-location

	auto *l = GetLuaState();
	for(auto &evName : {"ON_UPDATE_RENDER_DATA", "ON_RENDER_BUFFERS_INITIALIZED"})
		componentManager.RegisterEvent(evName, typeid(ecs::BaseEntity), ComponentEventInfo::Type::Broadcast);
}

pragma::BaseEntityComponent *pragma::CGame::CreateLuaEntityComponent(ecs::BaseEntity &ent, std::string classname) { return Game::CreateLuaEntityComponent<CLuaBaseEntityComponent, LuaCore::CLuaBaseEntityComponentHolder>(ent, classname); }
