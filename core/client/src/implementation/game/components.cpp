// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

// --template-include-location

module pragma.client;


import :game;
import :entities.components;

void CGame::InitializeEntityComponents(pragma::EntityComponentManager &componentManager)
{
	pragma::Game::InitializeEntityComponents(componentManager);
	CBaseEntity::RegisterEvents(componentManager);
	constexpr auto hideInEditor = pragma::ComponentRegInfo::Flags::HideInEditor;
	componentManager.RegisterComponentType<pragma::CAIComponent>("ai", {"ai", hideInEditor});
	componentManager.RegisterComponentType<pragma::CCharacterComponent>("character", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CColorComponent>("color", {"rendering"});
	componentManager.RegisterComponentType<pragma::CSurfaceComponent>("surface", {"world/bounds"});
	componentManager.RegisterComponentType<pragma::CScoreComponent>("score", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CFlammableComponent>("flammable", {"gameplay"});
	componentManager.RegisterComponentType<pragma::CHealthComponent>("health", {"gameplay"});
	componentManager.RegisterComponentType<pragma::CNameComponent>("name", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::CNetworkedComponent>("networked", {"networking", hideInEditor});
	componentManager.RegisterComponentType<pragma::CObservableComponent>("observable", {"gameplay"});
	componentManager.RegisterComponentType<pragma::CObserverComponent>("observer", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CPhysicsComponent>("physics", {"physics"});
	componentManager.RegisterComponentType<pragma::CPlayerComponent>("player", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRadiusComponent>("radius", {"world/bounds"});
	componentManager.RegisterComponentType<pragma::CFieldAngleComponent>("field_angle", {"world/bounds"});
	componentManager.RegisterComponentType<pragma::CRenderComponent>("render", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CSoundEmitterComponent>("sound_emitter", {"audio"});
	componentManager.RegisterComponentType<pragma::CToggleComponent>("toggle", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CTransformComponent>("transform", {"world/transform"});
	componentManager.RegisterComponentType<pragma::CVehicleComponent>("vehicle", {"gameplay"});
	componentManager.RegisterComponentType<pragma::CWeaponComponent>("weapon", {"gameplay"});
	componentManager.RegisterComponentType<pragma::CWheelComponent>("wheel", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CSoundDspComponent>("sound_dsp", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::CSoundDspChorusComponent>("sound_dsp_chorus", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::CSoundDspDistortionComponent>("sound_dsp_distortion", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::CSoundDspEAXReverbComponent>("sound_dsp_eax_reverb", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::CSoundDspEchoComponent>("sound_dsp_echo", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::CSoundDspEqualizerComponent>("sound_dsp_equalizer", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::CSoundDspFlangerComponent>("sound_dsp_flanger", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::CCameraComponent>("camera", {"rendering/camera"});
	componentManager.RegisterComponentType<pragma::CDecalComponent>("decal", {"rendering"});
	componentManager.RegisterComponentType<pragma::CExplosionComponent>("explosion", {"gameplay/effects"});
	componentManager.RegisterComponentType<pragma::CFireComponent>("fire", {"gameplay/effects"});
	componentManager.RegisterComponentType<pragma::CFogControllerComponent>("fog_controller", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::CLightComponent>("light", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::CLightDirectionalComponent>("light_directional", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::CLightPointComponent>("light_point", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::CLightSpotComponent>("light_spot", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::CLightSpotVolComponent>("light_spot_volume", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::CMicrophoneComponent>("microphone", {"audio"});
	componentManager.RegisterComponentType<pragma::ecs::CParticleSystemComponent>("particle_system", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::CQuakeComponent>("quake", {"gameplay/effects"});
	componentManager.RegisterComponentType<pragma::CSmokeTrailComponent>("smoke_trail", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::CSoundComponent>("sound", {"audio"});
	componentManager.RegisterComponentType<pragma::CSoundScapeComponent>("sound_scape", {"audio"});
	componentManager.RegisterComponentType<pragma::CSpriteComponent>("sprite", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::CEnvTimescaleComponent>("env_time_scale", {"world"});
	componentManager.RegisterComponentType<pragma::CWindComponent>("wind", {"world"});
	componentManager.RegisterComponentType<pragma::CBrushComponent>("brush", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::CKinematicComponent>("kinematic", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::CFuncPhysicsComponent>("func_physics", {"physics", hideInEditor});
	// componentManager.RegisterComponentType<pragma::CFuncPortalComponent>("func_portal", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::CButtonComponent>("button", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::CBotComponent>("bot", {"ai", hideInEditor});
	componentManager.RegisterComponentType<pragma::CPointConstraintBallSocketComponent>("constraint_ball_socket", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::CPointConstraintConeTwistComponent>("constraint_cone_twist", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::CPointConstraintDoFComponent>("constraint_dof", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::CPointConstraintFixedComponent>("constraint_fixed", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::CPointConstraintHingeComponent>("constraint_hinge", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::CPointConstraintSliderComponent>("constraint_slider", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::CPointTargetComponent>("target", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::CPropComponent>("prop", {"physics"});
	componentManager.RegisterComponentType<pragma::CPropDynamicComponent>("prop_dynamic", {"physics"});
	componentManager.RegisterComponentType<pragma::CPropPhysicsComponent>("prop_physics", {"physics"});
	componentManager.RegisterComponentType<pragma::CTouchComponent>("touch", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CSkyboxComponent>("skybox", {"world"});
	componentManager.RegisterComponentType<pragma::CWorldComponent>("world", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::ecs::CShooterComponent>("shooter", {"gameplay"});
	componentManager.RegisterComponentType<pragma::CModelComponent>("model", {"rendering/model"});
	componentManager.RegisterComponentType<pragma::CMaterialOverrideComponent>("material_override", {"rendering/model"});
	componentManager.RegisterComponentType<pragma::CMaterialPropertyOverrideComponent>("material_property_override", {"rendering/model"});
	componentManager.RegisterComponentType<pragma::CAnimatedComponent>("animated", {"animation"});
	componentManager.RegisterComponentType<pragma::CGenericComponent>("entity", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::CIOComponent>("io", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::CTimeScaleComponent>("time_scale", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::CAttachmentComponent>("attachment", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::CReflectionProbeComponent>("reflection_probe", {"rendering"});
	componentManager.RegisterComponentType<pragma::CPBRConverterComponent>("pbr_converter", {"util", hideInEditor});
	componentManager.RegisterComponentType<pragma::CShadowComponent>("shadow_map", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CShadowCSMComponent>("csm", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CShadowManagerComponent>("shadow_manager", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::COcclusionCullerComponent>("occlusion_culler", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CSceneComponent>("scene", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CGamemodeComponent>("gamemode", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CGameComponent>("game", {"gameplay", hideInEditor});

	componentManager.RegisterComponentType<pragma::CLiquidComponent>("liquid", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::CBuoyancyComponent>("buoyancy", {"physics"});
	componentManager.RegisterComponentType<pragma::CLiquidSurfaceComponent>("liquid_surface", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::CLiquidVolumeComponent>("liquid_volume", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::CWaterSurfaceComponent>("water_surface", {"physics/fluid", hideInEditor});

	componentManager.RegisterComponentType<pragma::CEnvSoundProbeComponent>("sound_probe", {"audio"});
	componentManager.RegisterComponentType<pragma::CWeatherComponent>("weather", {"rendering"});
	componentManager.RegisterComponentType<pragma::CFuncSoftPhysicsComponent>("func_soft_physics", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::CFlashlightComponent>("flashlight", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CListenerComponent>("listener", {"audio", hideInEditor});
	componentManager.RegisterComponentType<pragma::CViewBodyComponent>("view_body", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CViewModelComponent>("view_model", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CFlexComponent>("flex", {"animation"});
	componentManager.RegisterComponentType<pragma::CSoftBodyComponent>("softbody", {"physics"});
	componentManager.RegisterComponentType<pragma::CVertexAnimatedComponent>("vertex_animated", {"animation"});
	componentManager.RegisterComponentType<pragma::CSkyCameraComponent>("sky_camera", {"rendering/camera"});
	componentManager.RegisterComponentType<pragma::COwnableComponent>("ownable", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::CDebugTextComponent>("debug_text", {"debug"});
	componentManager.RegisterComponentType<pragma::CDebugPointComponent>("debug_point", {"debug"});
	componentManager.RegisterComponentType<pragma::CDebugLineComponent>("debug_line", {"debug"});
	componentManager.RegisterComponentType<pragma::CDebugBoxComponent>("debug_box", {"debug"});
	componentManager.RegisterComponentType<pragma::CDebugSphereComponent>("debug_sphere", {"debug"});
	componentManager.RegisterComponentType<pragma::CDebugConeComponent>("debug_cone", {"debug"});
	componentManager.RegisterComponentType<pragma::CDebugCylinderComponent>("debug_cylinder", {"debug"});
	componentManager.RegisterComponentType<pragma::CDebugPlaneComponent>("debug_plane", {"debug"});
	componentManager.RegisterComponentType<pragma::CBSPLeafComponent>("bsp_leaf", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::CPointAtTargetComponent>("point_at_target", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRaytracingComponent>("raytracing", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CEyeComponent>("eye", {"rendering/model"});
	componentManager.RegisterComponentType<pragma::CBvhComponent>("bvh", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<pragma::CAnimatedBvhComponent>("animated_bvh", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<pragma::CStaticBvhCacheComponent>("static_bvh_cache", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<pragma::CStaticBvhUserComponent>("static_bvh_user", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<pragma::COpticalCameraComponent>("optical_camera", {"rendering/camera"});
	componentManager.RegisterComponentType<pragma::CHitboxBvhComponent>("hitbox_bvh", {"rendering/bvh", hideInEditor});
	componentManager.RegisterComponentType<pragma::CChildComponent>("child", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::CInputComponent>("input", {"core", hideInEditor});

	componentManager.RegisterComponentType<pragma::CRendererPpFogComponent>("renderer_pp_fog", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRendererPpDoFComponent>("renderer_pp_dof", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRendererPpBloomComponent>("renderer_pp_bloom", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRendererPpToneMappingComponent>("renderer_pp_tone_mapping", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRendererPpFxaaComponent>("renderer_pp_fxaa", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRendererPpMotionBlurComponent>("renderer_pp_motion_blur", {"rendering/post_processing", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRendererPpVolumetricComponent>("renderer_pp_volumetric", {"rendering/post_processing", hideInEditor});

	componentManager.RegisterComponentType<pragma::CMotionBlurDataComponent>("motion_blur_data", {"rendering/camera"});
	componentManager.RegisterComponentType<pragma::CBSPComponent>("bsp", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::CLightMapComponent>("light_map", {"rendering/lighting", hideInEditor});
	componentManager.RegisterComponentType<pragma::CLightMapReceiverComponent>("light_map_receiver", {"rendering/lighting", hideInEditor});
	componentManager.RegisterComponentType<pragma::CLightMapDataCacheComponent>("light_map_data_cache", {"rendering/lighting", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRendererComponent>("renderer", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRasterizationRendererComponent>("rasterization_renderer", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CRaytracingRendererComponent>("raytracing_renderer", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::CLiquidControlComponent>("liquid_control", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::CLiquidSurfaceSimulationComponent>("liquid_surface_simulation", {"physics/fluid", hideInEditor});

	componentManager.RegisterComponentType<pragma::CGlobalShaderInputComponent>("global_shader_input", {"rendering"});

	componentManager.RegisterComponentType<pragma::CDebugHitboxComponent>("debug_hitbox", {"debug"});
	// --template-component-register-location

	auto *l = GetLuaState();
	for(auto &evName : {"ON_UPDATE_RENDER_DATA", "ON_RENDER_BUFFERS_INITIALIZED"})
		componentManager.RegisterEvent(evName, typeid(pragma::ecs::BaseEntity), pragma::ComponentEventInfo::Type::Broadcast);
}

pragma::BaseEntityComponent *CGame::CreateLuaEntityComponent(pragma::ecs::BaseEntity &ent, std::string classname) { return pragma::Game::CreateLuaEntityComponent<pragma::CLuaBaseEntityComponent, pragma::lua::CLuaBaseEntityComponentHolder>(ent, classname); }
