/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/entities/components/c_lua_component.hpp"
#include "pragma/entities/components/c_ownable_component.hpp"
#include <pragma/lua/lentity_component_lua.hpp>
#include <pragma/game/game_entity_component.hpp>
#include <pragma/entities/entity_component_manager.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_surface_component.hpp"
#include "pragma/entities/components/c_score_component.hpp"
#include "pragma/entities/components/c_flammable_component.hpp"
#include "pragma/entities/components/c_health_component.hpp"
#include "pragma/entities/components/c_name_component.hpp"
#include "pragma/entities/components/c_networked_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/entities/components/c_observer_component.hpp"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_field_angle_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_vehicle_component.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_raytracing_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_animated_bvh_component.hpp"
#include "pragma/entities/components/c_static_bvh_cache_component.hpp"
#include "pragma/entities/components/c_static_bvh_user_component.hpp"
#include "pragma/entities/components/c_light_map_data_cache_component.hpp"
#include "pragma/entities/components/c_optical_camera_component.hpp"
#include "pragma/entities/components/c_hitbox_bvh_component.hpp"
#include "pragma/entities/components/c_input_component.hpp"
#include "pragma/entities/components/liquid/c_buoyancy_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_volume_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_control_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_simulation_component.hpp"
#include "pragma/entities/components/c_debug_hitbox_component.hpp"
// --template-include-location
#include "pragma/entities/environment/audio/c_env_sound_dsp.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_equalizer.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_flanger.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_explosion.h"
#include "pragma/entities/environment/effects/c_env_fire.h"
#include "pragma/entities/environment/c_env_decal.h"
#include "pragma/entities/environment/c_env_fog_controller.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/environment/c_env_microphone.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/c_env_quake.h"
#include "pragma/entities/environment/effects/c_env_smoke_trail.h"
#include "pragma/entities/environment/audio/c_env_sound.h"
#include "pragma/entities/environment/audio/c_env_soundscape.h"
#include "pragma/entities/environment/effects/c_env_sprite.h"
#include "pragma/entities/environment/c_env_weather.h"
#include "pragma/entities/environment/c_sky_camera.hpp"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/environment/c_env_wind.hpp"
#include "pragma/entities/func/c_func_brush.h"
#include "pragma/entities/func/c_func_kinematic.hpp"
#include "pragma/entities/func/c_func_physics.h"
#include "pragma/entities/func/c_func_softphysics.hpp"
#include "pragma/entities/func/c_func_portal.h"
#include "pragma/entities/components/liquid/c_liquid_component.hpp"
#include "pragma/entities/func/c_funcbutton.h"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/c_bot.h"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/point/constraints/c_point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/c_point_constraint_dof.h"
#include "pragma/entities/point/constraints/c_point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/c_point_constraint_fixed.h"
#include "pragma/entities/point/constraints/c_point_constraint_hinge.h"
#include "pragma/entities/point/constraints/c_point_constraint_slider.h"
#include "pragma/entities/point/c_point_rendertarget.h"
#include "pragma/entities/point/c_point_target.h"
#include "pragma/entities/prop/c_prop_base.hpp"
#include "pragma/entities/prop/c_prop_dynamic.hpp"
#include "pragma/entities/prop/c_prop_physics.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/c_skybox.h"
#include "pragma/entities/c_flashlight.h"
#include "pragma/entities/components/c_shooter_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/components/c_io_component.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/components/c_attachment_component.hpp"
#include "pragma/entities/components/c_child_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/entities/components/c_point_at_target_component.hpp"
#include "pragma/entities/components/c_gamemode_component.hpp"
#include "pragma/entities/components/c_game_component.hpp"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/c_wheel.hpp"
#include "pragma/entities/environment/c_env_timescale.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/environment/audio/c_env_sound_probe.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/c_water_surface.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/entities/components/c_softbody_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_debug_component.hpp"
#include "pragma/entities/components/c_bsp_leaf_component.hpp"
#include "pragma/entities/components/renderers/c_raytracing_renderer_component.hpp"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/entities/components/c_bsp_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_fog_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_dof_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_bloom_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_glow_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_tone_mapping_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_fxaa_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_motion_blur_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_volumetric_component.hpp"
#include "pragma/entities/components/c_motion_blur_data_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

void CGame::InitializeEntityComponents(pragma::EntityComponentManager &componentManager)
{
	Game::InitializeEntityComponents(componentManager);
	CBaseEntity::RegisterEvents(componentManager);
	componentManager.RegisterComponentType<pragma::CAIComponent>("ai");
	componentManager.RegisterComponentType<pragma::CCharacterComponent>("character");
	componentManager.RegisterComponentType<pragma::CColorComponent>("color");
	componentManager.RegisterComponentType<pragma::CSurfaceComponent>("surface");
	componentManager.RegisterComponentType<pragma::CScoreComponent>("score");
	componentManager.RegisterComponentType<pragma::CFlammableComponent>("flammable");
	componentManager.RegisterComponentType<pragma::CHealthComponent>("health");
	componentManager.RegisterComponentType<pragma::CNameComponent>("name");
	componentManager.RegisterComponentType<pragma::CNetworkedComponent>("networked");
	componentManager.RegisterComponentType<pragma::CObservableComponent>("observable");
	componentManager.RegisterComponentType<pragma::CObserverComponent>("observer");
	componentManager.RegisterComponentType<pragma::CPhysicsComponent>("physics");
	componentManager.RegisterComponentType<pragma::CPlayerComponent>("player");
	componentManager.RegisterComponentType<pragma::CRadiusComponent>("radius");
	componentManager.RegisterComponentType<pragma::CFieldAngleComponent>("field_angle");
	componentManager.RegisterComponentType<pragma::CRenderComponent>("render");
	componentManager.RegisterComponentType<pragma::CSoundEmitterComponent>("sound_emitter");
	componentManager.RegisterComponentType<pragma::CToggleComponent>("toggle");
	componentManager.RegisterComponentType<pragma::CTransformComponent>("transform");
	componentManager.RegisterComponentType<pragma::CVehicleComponent>("vehicle");
	componentManager.RegisterComponentType<pragma::CWeaponComponent>("weapon");
	componentManager.RegisterComponentType<pragma::CWheelComponent>("wheel");
	componentManager.RegisterComponentType<pragma::CSoundDspComponent>("sound_dsp");
	componentManager.RegisterComponentType<pragma::CSoundDspChorusComponent>("sound_dsp_chorus");
	componentManager.RegisterComponentType<pragma::CSoundDspDistortionComponent>("sound_dsp_distortion");
	componentManager.RegisterComponentType<pragma::CSoundDspEAXReverbComponent>("sound_dsp_eax_reverb");
	componentManager.RegisterComponentType<pragma::CSoundDspEchoComponent>("sound_dsp_echo");
	componentManager.RegisterComponentType<pragma::CSoundDspEqualizerComponent>("sound_dsp_equalizer");
	componentManager.RegisterComponentType<pragma::CSoundDspFlangerComponent>("sound_dsp_flanger");
	componentManager.RegisterComponentType<pragma::CCameraComponent>("camera");
	componentManager.RegisterComponentType<pragma::CDecalComponent>("decal");
	componentManager.RegisterComponentType<pragma::CExplosionComponent>("explosion");
	componentManager.RegisterComponentType<pragma::CFireComponent>("fire");
	componentManager.RegisterComponentType<pragma::CFogControllerComponent>("fog_controller");
	componentManager.RegisterComponentType<pragma::CLightComponent>("light");
	componentManager.RegisterComponentType<pragma::CLightDirectionalComponent>("light_directional");
	componentManager.RegisterComponentType<pragma::CLightPointComponent>("light_point");
	componentManager.RegisterComponentType<pragma::CLightSpotComponent>("light_spot");
	componentManager.RegisterComponentType<pragma::CLightSpotVolComponent>("light_spot_volume");
	componentManager.RegisterComponentType<pragma::CMicrophoneComponent>("microphone");
	componentManager.RegisterComponentType<pragma::CParticleSystemComponent>("particle_system");
	componentManager.RegisterComponentType<pragma::CQuakeComponent>("quake");
	componentManager.RegisterComponentType<pragma::CSmokeTrailComponent>("smoke_trail");
	componentManager.RegisterComponentType<pragma::CSoundComponent>("sound");
	componentManager.RegisterComponentType<pragma::CSoundScapeComponent>("sound_scape");
	componentManager.RegisterComponentType<pragma::CSpriteComponent>("sprite");
	componentManager.RegisterComponentType<pragma::CEnvTimescaleComponent>("env_time_scale");
	componentManager.RegisterComponentType<pragma::CWindComponent>("wind");
	componentManager.RegisterComponentType<pragma::CBrushComponent>("brush");
	componentManager.RegisterComponentType<pragma::CKinematicComponent>("kinematic");
	componentManager.RegisterComponentType<pragma::CFuncPhysicsComponent>("func_physics");
	// componentManager.RegisterComponentType<pragma::CFuncPortalComponent>("func_portal");
	componentManager.RegisterComponentType<pragma::CButtonComponent>("button");
	componentManager.RegisterComponentType<pragma::CBotComponent>("bot");
	componentManager.RegisterComponentType<pragma::CPointConstraintBallSocketComponent>("constraint_ball_socket");
	componentManager.RegisterComponentType<pragma::CPointConstraintConeTwistComponent>("constraint_cone_twist");
	componentManager.RegisterComponentType<pragma::CPointConstraintDoFComponent>("constraint_dof");
	componentManager.RegisterComponentType<pragma::CPointConstraintFixedComponent>("constraint_fixed");
	componentManager.RegisterComponentType<pragma::CPointConstraintHingeComponent>("constraint_hinge");
	componentManager.RegisterComponentType<pragma::CPointConstraintSliderComponent>("constraint_slider");
	componentManager.RegisterComponentType<pragma::CPointTargetComponent>("target");
	componentManager.RegisterComponentType<pragma::CPropComponent>("prop");
	componentManager.RegisterComponentType<pragma::CPropDynamicComponent>("prop_dynamic");
	componentManager.RegisterComponentType<pragma::CPropPhysicsComponent>("prop_physics");
	componentManager.RegisterComponentType<pragma::CTouchComponent>("touch");
	componentManager.RegisterComponentType<pragma::CSkyboxComponent>("skybox");
	componentManager.RegisterComponentType<pragma::CWorldComponent>("world");
	componentManager.RegisterComponentType<pragma::CShooterComponent>("shooter");
	componentManager.RegisterComponentType<pragma::CModelComponent>("model");
	componentManager.RegisterComponentType<pragma::CAnimatedComponent>("animated");
	componentManager.RegisterComponentType<pragma::CGenericComponent>("entity");
	componentManager.RegisterComponentType<pragma::CIOComponent>("io");
	componentManager.RegisterComponentType<pragma::CTimeScaleComponent>("time_scale");
	componentManager.RegisterComponentType<pragma::CAttachmentComponent>("attachment");
	componentManager.RegisterComponentType<pragma::CReflectionProbeComponent>("reflection_probe");
	componentManager.RegisterComponentType<pragma::CPBRConverterComponent>("pbr_converter");
	componentManager.RegisterComponentType<pragma::CShadowComponent>("shadow_map");
	componentManager.RegisterComponentType<pragma::CShadowCSMComponent>("csm");
	componentManager.RegisterComponentType<pragma::CShadowManagerComponent>("shadow_manager");
	componentManager.RegisterComponentType<pragma::COcclusionCullerComponent>("occlusion_culler");
	componentManager.RegisterComponentType<pragma::CSceneComponent>("scene");
	componentManager.RegisterComponentType<pragma::CGamemodeComponent>("gamemode");
	componentManager.RegisterComponentType<pragma::CGameComponent>("game");

	componentManager.RegisterComponentType<pragma::CLiquidComponent>("liquid");
	componentManager.RegisterComponentType<pragma::CBuoyancyComponent>("buoyancy");
	componentManager.RegisterComponentType<pragma::CLiquidSurfaceComponent>("liquid_surface");
	componentManager.RegisterComponentType<pragma::CLiquidVolumeComponent>("liquid_volume");
	componentManager.RegisterComponentType<pragma::CWaterSurfaceComponent>("water_surface");

	componentManager.RegisterComponentType<pragma::CEnvSoundProbeComponent>("sound_probe");
	componentManager.RegisterComponentType<pragma::CWeatherComponent>("weather");
	componentManager.RegisterComponentType<pragma::CFuncSoftPhysicsComponent>("func_soft_physics");
	componentManager.RegisterComponentType<pragma::CFlashlightComponent>("flashlight");
	componentManager.RegisterComponentType<pragma::CListenerComponent>("listener");
	componentManager.RegisterComponentType<pragma::CViewBodyComponent>("view_body");
	componentManager.RegisterComponentType<pragma::CViewModelComponent>("view_model");
	componentManager.RegisterComponentType<pragma::CFlexComponent>("flex");
	componentManager.RegisterComponentType<pragma::CSoftBodyComponent>("softbody");
	componentManager.RegisterComponentType<pragma::CVertexAnimatedComponent>("vertex_animated");
	componentManager.RegisterComponentType<pragma::CSkyCameraComponent>("sky_camera");
	componentManager.RegisterComponentType<pragma::COwnableComponent>("ownable");
	componentManager.RegisterComponentType<pragma::CDebugTextComponent>("debug_text");
	componentManager.RegisterComponentType<pragma::CDebugPointComponent>("debug_point");
	componentManager.RegisterComponentType<pragma::CDebugLineComponent>("debug_line");
	componentManager.RegisterComponentType<pragma::CDebugBoxComponent>("debug_box");
	componentManager.RegisterComponentType<pragma::CDebugSphereComponent>("debug_sphere");
	componentManager.RegisterComponentType<pragma::CDebugConeComponent>("debug_cone");
	componentManager.RegisterComponentType<pragma::CDebugCylinderComponent>("debug_cylinder");
	componentManager.RegisterComponentType<pragma::CDebugPlaneComponent>("debug_plane");
	componentManager.RegisterComponentType<pragma::CBSPLeafComponent>("bsp_leaf");
	componentManager.RegisterComponentType<pragma::CPointAtTargetComponent>("point_at_target");
	componentManager.RegisterComponentType<pragma::CRaytracingComponent>("raytracing");
	componentManager.RegisterComponentType<pragma::CEyeComponent>("eye");
	componentManager.RegisterComponentType<pragma::CBvhComponent>("bvh");
	componentManager.RegisterComponentType<pragma::CAnimatedBvhComponent>("animated_bvh");
	componentManager.RegisterComponentType<pragma::CStaticBvhCacheComponent>("static_bvh_cache");
	componentManager.RegisterComponentType<pragma::CStaticBvhUserComponent>("static_bvh_user");
	componentManager.RegisterComponentType<pragma::COpticalCameraComponent>("optical_camera");
	componentManager.RegisterComponentType<pragma::CHitboxBvhComponent>("hitbox_bvh");
	componentManager.RegisterComponentType<pragma::CChildComponent>("child");
	componentManager.RegisterComponentType<pragma::CInputComponent>("input");

	componentManager.RegisterComponentType<pragma::CRendererPpFogComponent>("renderer_pp_fog");
	componentManager.RegisterComponentType<pragma::CRendererPpDoFComponent>("renderer_pp_dof");
	componentManager.RegisterComponentType<pragma::CRendererPpBloomComponent>("renderer_pp_bloom");
	componentManager.RegisterComponentType<pragma::CRendererPpGlowComponent>("renderer_pp_glow");
	componentManager.RegisterComponentType<pragma::CRendererPpToneMappingComponent>("renderer_pp_tone_mapping");
	componentManager.RegisterComponentType<pragma::CRendererPpFxaaComponent>("renderer_pp_fxaa");
	componentManager.RegisterComponentType<pragma::CRendererPpMotionBlurComponent>("renderer_pp_motion_blur");
	componentManager.RegisterComponentType<pragma::CRendererPpVolumetricComponent>("renderer_pp_volumetric");

	componentManager.RegisterComponentType<pragma::CMotionBlurDataComponent>("motion_blur_data");
	componentManager.RegisterComponentType<pragma::CBSPComponent>("bsp");
	componentManager.RegisterComponentType<pragma::CLightMapComponent>("light_map");
	componentManager.RegisterComponentType<pragma::CLightMapReceiverComponent>("light_map_receiver");
	componentManager.RegisterComponentType<pragma::CLightMapDataCacheComponent>("light_map_data_cache");
	componentManager.RegisterComponentType<pragma::CRendererComponent>("renderer");
	componentManager.RegisterComponentType<pragma::CRasterizationRendererComponent>("rasterization_renderer");
	componentManager.RegisterComponentType<pragma::CRaytracingRendererComponent>("raytracing_renderer");
	componentManager.RegisterComponentType<pragma::CLiquidControlComponent>("liquid_control");
	componentManager.RegisterComponentType<pragma::CLiquidSurfaceSimulationComponent>("liquid_surface_simulation");

	componentManager.RegisterComponentType<pragma::CDebugHitboxComponent>("debug_hitbox");
	// --template-component-register-location

	auto *l = GetLuaState();
	for(auto &evName : {"ON_UPDATE_RENDER_DATA", "ON_RENDER_BUFFERS_INITIALIZED"})
		componentManager.RegisterEvent(evName, typeid(BaseEntity), pragma::ComponentEventInfo::Type::Broadcast);
}

pragma::BaseEntityComponent *CGame::CreateLuaEntityComponent(BaseEntity &ent, std::string classname) { return Game::CreateLuaEntityComponent<pragma::CLuaBaseEntityComponent, pragma::lua::CLuaBaseEntityComponentHolder>(ent, classname); }
