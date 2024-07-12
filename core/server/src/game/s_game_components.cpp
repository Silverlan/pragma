/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lua_component.hpp"
#include "pragma/entities/components/s_ownable_component.hpp"
#include "pragma/entities/components/s_debug_component.hpp"
#include <pragma/lua/lentity_component_lua.hpp>
#include <pragma/game/game_entity_component.hpp>
#include <pragma/entities/entity_component_manager.hpp>
#include <luainterface.hpp>
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_color_component.hpp"
#include "pragma/entities/components/s_score_component.hpp"
#include "pragma/entities/components/s_flammable_component.hpp"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/entities/components/s_name_component.hpp"
#include "pragma/entities/components/s_networked_component.hpp"
#include "pragma/entities/components/s_observable_component.hpp"
#include "pragma/entities/components/s_observer_component.hpp"
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
#include "pragma/entities/components/s_time_scale_component.hpp"
#include "pragma/entities/environment/s_env_wind.hpp"
#include "pragma/entities/filter/s_filter_entity_class.h"
#include "pragma/entities/filter/s_filter_entity_name.h"
#include "pragma/entities/func/s_func_brush.h"
#include "pragma/entities/func/s_func_kinematic.hpp"
#include "pragma/entities/func/s_func_physics.h"
#include "pragma/entities/func/s_func_softphysics.hpp"
#include "pragma/entities/func/s_func_portal.h"
#include "pragma/entities/components/liquid/s_liquid_component.hpp"
#include "pragma/entities/func/s_funcbutton.h"
#include "pragma/entities/game_player_spawn.h"
#include "pragma/entities/logic/s_logic_relay.h"
#include "pragma/entities/s_bot.h"
#include "pragma/entities/point/constraints/s_point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/s_point_constraint_dof.h"
#include "pragma/entities/point/constraints/s_point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/s_point_constraint_fixed.h"
#include "pragma/entities/point/constraints/s_point_constraint_hinge.h"
#include "pragma/entities/point/constraints/s_point_constraint_slider.h"
#include "pragma/entities/point/s_point_path_node.h"
#include "pragma/entities/point/s_point_rendertarget.h"
#include "pragma/entities/point/s_point_target.h"
#include "pragma/entities/prop/s_prop_base.hpp"
#include "pragma/entities/prop/s_prop_dynamic.hpp"
#include "pragma/entities/prop/s_prop_physics.h"
#include "pragma/entities/trigger/s_trigger_gravity.h"
#include "pragma/entities/trigger/s_trigger_hurt.h"
#include "pragma/entities/trigger/s_trigger_push.h"
#include "pragma/entities/trigger/s_trigger_remove.h"
#include "pragma/entities/trigger/s_trigger_teleport.h"
#include "pragma/entities/trigger/s_trigger_touch.h"
#include "pragma/entities/s_skybox.h"
#include "pragma/entities/s_flashlight.h"
#include "pragma/entities/components/s_shooter_component.hpp"
#include "pragma/entities/components/s_model_component.hpp"
#include "pragma/entities/components/s_animated_component.hpp"
#include "pragma/entities/components/s_entity_component.hpp"
#include "pragma/entities/components/s_io_component.hpp"
#include "pragma/entities/components/s_time_scale_component.hpp"
#include "pragma/entities/info/s_info_landmark.hpp"
#include "pragma/entities/components/s_attachment_component.hpp"
#include "pragma/entities/components/s_child_component.hpp"
#include "pragma/entities/components/s_generic_component.hpp"
#include "pragma/entities/components/s_point_at_target_component.hpp"
#include "pragma/entities/components/s_gamemode_component.hpp"
#include "pragma/entities/components/s_game_component.hpp"
#include "pragma/entities/components/s_surface_component.hpp"
#include "pragma/entities/components/liquid/s_buoyancy_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_surface_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_volume_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_control_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_surface_simulation_component.hpp"
// --template-include-location
#include "pragma/entities/environment/s_env_timescale.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

void SGame::InitializeEntityComponents(pragma::EntityComponentManager &componentManager)
{
	Game::InitializeEntityComponents(componentManager);
	componentManager.RegisterComponentType<pragma::SAIComponent>("ai");
	componentManager.RegisterComponentType<pragma::SCharacterComponent>("character");
	componentManager.RegisterComponentType<pragma::SColorComponent>("color");
	componentManager.RegisterComponentType<pragma::SSurfaceComponent>("surface");
	componentManager.RegisterComponentType<pragma::SScoreComponent>("score");
	componentManager.RegisterComponentType<pragma::SFlammableComponent>("flammable");
	componentManager.RegisterComponentType<pragma::SHealthComponent>("health");
	componentManager.RegisterComponentType<pragma::SNameComponent>("name");
	componentManager.RegisterComponentType<pragma::SNetworkedComponent>("networked");
	componentManager.RegisterComponentType<pragma::SObservableComponent>("observable");
	componentManager.RegisterComponentType<pragma::SObserverComponent>("observer");
	componentManager.RegisterComponentType<pragma::SPhysicsComponent>("physics");
	componentManager.RegisterComponentType<pragma::SPlayerComponent>("player");
	componentManager.RegisterComponentType<pragma::SRadiusComponent>("radius");
	componentManager.RegisterComponentType<pragma::SFieldAngleComponent>("field_angle");
	componentManager.RegisterComponentType<pragma::SRenderComponent>("render");
	componentManager.RegisterComponentType<pragma::SSoundEmitterComponent>("sound_emitter");
	componentManager.RegisterComponentType<pragma::SToggleComponent>("toggle");
	componentManager.RegisterComponentType<pragma::STransformComponent>("transform");
	componentManager.RegisterComponentType<pragma::SVehicleComponent>("vehicle");
	componentManager.RegisterComponentType<pragma::SWeaponComponent>("weapon");
	componentManager.RegisterComponentType<pragma::SWheelComponent>("wheel");
	componentManager.RegisterComponentType<pragma::SSoundDspComponent>("sound_dsp");
	componentManager.RegisterComponentType<pragma::SSoundDspChorusComponent>("sound_dsp_chorus");
	componentManager.RegisterComponentType<pragma::SSoundDspDistortionComponent>("sound_dsp_distortion");
	componentManager.RegisterComponentType<pragma::SSoundDspEAXReverbComponent>("sound_dsp_eax_reverb");
	componentManager.RegisterComponentType<pragma::SSoundDspEchoComponent>("sound_dsp_echo");
	componentManager.RegisterComponentType<pragma::SSoundDspEqualizerComponent>("sound_dsp_equalizer");
	componentManager.RegisterComponentType<pragma::SSoundDspFlangerComponent>("sound_dsp_flanger");
	componentManager.RegisterComponentType<pragma::SCameraComponent>("camera");
	componentManager.RegisterComponentType<pragma::SDecalComponent>("decal");
	componentManager.RegisterComponentType<pragma::SExplosionComponent>("explosion");
	componentManager.RegisterComponentType<pragma::SFireComponent>("fire");
	componentManager.RegisterComponentType<pragma::SFogControllerComponent>("fog_controller");
	componentManager.RegisterComponentType<pragma::SLightComponent>("light");
	componentManager.RegisterComponentType<pragma::SLightDirectionalComponent>("light_directional");
	componentManager.RegisterComponentType<pragma::SLightPointComponent>("light_point");
	componentManager.RegisterComponentType<pragma::SLightSpotComponent>("light_spot");
	componentManager.RegisterComponentType<pragma::SLightSpotVolComponent>("light_spot_volume");
	componentManager.RegisterComponentType<pragma::SMicrophoneComponent>("microphone");
	componentManager.RegisterComponentType<pragma::SParticleSystemComponent>("particle_system");
	componentManager.RegisterComponentType<pragma::SQuakeComponent>("quake");
	componentManager.RegisterComponentType<pragma::SSmokeTrailComponent>("smoke_trail");
	componentManager.RegisterComponentType<pragma::SSoundComponent>("sound");
	componentManager.RegisterComponentType<pragma::SSoundScapeComponent>("sound_scape");
	componentManager.RegisterComponentType<pragma::SSpriteComponent>("sprite");
	componentManager.RegisterComponentType<pragma::SEnvTimescaleComponent>("env_time_scale");
	componentManager.RegisterComponentType<pragma::SWindComponent>("wind");
	componentManager.RegisterComponentType<pragma::SFilterClassComponent>("filter_class");
	componentManager.RegisterComponentType<pragma::SFilterNameComponent>("filter_name");
	componentManager.RegisterComponentType<pragma::SBrushComponent>("brush");
	componentManager.RegisterComponentType<pragma::SKinematicComponent>("kinematic");
	componentManager.RegisterComponentType<pragma::SFuncPhysicsComponent>("func_physics");
	componentManager.RegisterComponentType<pragma::SFuncSoftPhysicsComponent>("func_soft_physics");
	// componentManager.RegisterComponentType<pragma::SFuncPortalComponent>("func_portal");
	componentManager.RegisterComponentType<pragma::SButtonComponent>("button");
	componentManager.RegisterComponentType<pragma::SPlayerSpawnComponent>("player_spawn");
	componentManager.RegisterComponentType<pragma::SLogicRelayComponent>("relay");
	componentManager.RegisterComponentType<pragma::SBotComponent>("bot");
	componentManager.RegisterComponentType<pragma::SPointConstraintBallSocketComponent>("constraint_ball_socket");
	componentManager.RegisterComponentType<pragma::SPointConstraintConeTwistComponent>("constraint_cone_twist");
	componentManager.RegisterComponentType<pragma::SPointConstraintDoFComponent>("constraint_dof");
	componentManager.RegisterComponentType<pragma::SPointConstraintFixedComponent>("constraint_fixed");
	componentManager.RegisterComponentType<pragma::SPointConstraintHingeComponent>("constraint_hinge");
	componentManager.RegisterComponentType<pragma::SPointConstraintSliderComponent>("constraint_slider");
	componentManager.RegisterComponentType<pragma::SPathNodeComponent>("path_node");
	// componentManager.RegisterComponentType<pragma::SRenderTargetComponent>("render_target");
	componentManager.RegisterComponentType<pragma::SPointTargetComponent>("target");
	componentManager.RegisterComponentType<pragma::PropComponent>("prop");
	componentManager.RegisterComponentType<pragma::SPropDynamicComponent>("prop_dynamic");
	componentManager.RegisterComponentType<pragma::SPropPhysicsComponent>("prop_physics");
	componentManager.RegisterComponentType<pragma::STriggerGravityComponent>("trigger_gravity");
	componentManager.RegisterComponentType<pragma::STriggerHurtComponent>("trigger_hurt");
	componentManager.RegisterComponentType<pragma::STriggerPushComponent>("trigger_push");
	componentManager.RegisterComponentType<pragma::STriggerRemoveComponent>("trigger_remove");
	componentManager.RegisterComponentType<pragma::STriggerTeleportComponent>("trigger_teleport");
	componentManager.RegisterComponentType<pragma::STouchComponent>("touch");
	componentManager.RegisterComponentType<pragma::SSkyboxComponent>("skybox");
	componentManager.RegisterComponentType<pragma::SWorldComponent>("world");
	componentManager.RegisterComponentType<pragma::SFlashlightComponent>("flashlight");
	componentManager.RegisterComponentType<pragma::SShooterComponent>("shooter");
	componentManager.RegisterComponentType<pragma::SModelComponent>("model");
	componentManager.RegisterComponentType<pragma::SAnimatedComponent>("animated");
	componentManager.RegisterComponentType<pragma::SGenericComponent>("entity");
	componentManager.RegisterComponentType<pragma::SIOComponent>("io");
	componentManager.RegisterComponentType<pragma::STimeScaleComponent>("time_scale");
	componentManager.RegisterComponentType<pragma::SInfoLandmarkComponent>("info_landmark");
	componentManager.RegisterComponentType<pragma::SAttachmentComponent>("attachment");
	componentManager.RegisterComponentType<pragma::SChildComponent>("child");
	componentManager.RegisterComponentType<pragma::SOwnableComponent>("ownable");
	componentManager.RegisterComponentType<pragma::SDebugTextComponent>("debug_text");
	componentManager.RegisterComponentType<pragma::SDebugPointComponent>("debug_point");
	componentManager.RegisterComponentType<pragma::SDebugLineComponent>("debug_line");
	componentManager.RegisterComponentType<pragma::SDebugBoxComponent>("debug_box");
	componentManager.RegisterComponentType<pragma::SDebugSphereComponent>("debug_sphere");
	componentManager.RegisterComponentType<pragma::SDebugConeComponent>("debug_cone");
	componentManager.RegisterComponentType<pragma::SDebugCylinderComponent>("debug_cylinder");
	componentManager.RegisterComponentType<pragma::SDebugPlaneComponent>("debug_plane");
	componentManager.RegisterComponentType<pragma::SPointAtTargetComponent>("point_at_target");
	componentManager.RegisterComponentType<pragma::SGamemodeComponent>("gamemode");
	componentManager.RegisterComponentType<pragma::SGameComponent>("game");

	componentManager.RegisterComponentType<pragma::SLiquidComponent>("liquid");
	componentManager.RegisterComponentType<pragma::SBuoyancyComponent>("buoyancy");
	componentManager.RegisterComponentType<pragma::SLiquidSurfaceComponent>("liquid_surface");
	componentManager.RegisterComponentType<pragma::SLiquidVolumeComponent>("liquid_volume");
	componentManager.RegisterComponentType<pragma::SLiquidControlComponent>("liquid_control");
	componentManager.RegisterComponentType<pragma::SLiquidSurfaceSimulationComponent>("liquid_surface_simulation");
	// --template-component-register-location
}

pragma::BaseEntityComponent *SGame::CreateLuaEntityComponent(BaseEntity &ent, std::string classname) { return Game::CreateLuaEntityComponent<pragma::SLuaBaseEntityComponent, pragma::lua::SLuaBaseEntityComponentHolder>(ent, classname); }
