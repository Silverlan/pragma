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
	constexpr auto hideInEditor = pragma::ComponentRegInfo::Flags::HideInEditor;
	componentManager.RegisterComponentType<pragma::SAIComponent>("ai", {"ai", hideInEditor});
	componentManager.RegisterComponentType<pragma::SCharacterComponent>("character", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SColorComponent>("color", {"rendering"});
	componentManager.RegisterComponentType<pragma::SSurfaceComponent>("surface", {"world/bounds"});
	componentManager.RegisterComponentType<pragma::SScoreComponent>("score", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SFlammableComponent>("flammable", {"gameplay"});
	componentManager.RegisterComponentType<pragma::SHealthComponent>("health", {"gameplay"});
	componentManager.RegisterComponentType<pragma::SNameComponent>("name", {"name", hideInEditor});
	componentManager.RegisterComponentType<pragma::SNetworkedComponent>("networked", {"networking", hideInEditor});
	componentManager.RegisterComponentType<pragma::SObservableComponent>("observable", {"gameplay"});
	componentManager.RegisterComponentType<pragma::SObserverComponent>("observer", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SPhysicsComponent>("physics", {"physics"});
	componentManager.RegisterComponentType<pragma::SPlayerComponent>("player", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SRadiusComponent>("radius", {"world/bounds"});
	componentManager.RegisterComponentType<pragma::SFieldAngleComponent>("field_angle", {"world/bounds"});
	componentManager.RegisterComponentType<pragma::SRenderComponent>("render", {"rendering", hideInEditor});
	componentManager.RegisterComponentType<pragma::SSoundEmitterComponent>("sound_emitter", {"audio"});
	componentManager.RegisterComponentType<pragma::SToggleComponent>("toggle", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::STransformComponent>("transform", {"world/transform"});
	componentManager.RegisterComponentType<pragma::SVehicleComponent>("vehicle", {"gameplay"});
	componentManager.RegisterComponentType<pragma::SWeaponComponent>("weapon", {"gameplay"});
	componentManager.RegisterComponentType<pragma::SWheelComponent>("wheel", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SSoundDspComponent>("sound_dsp", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::SSoundDspChorusComponent>("sound_dsp_chorus", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::SSoundDspDistortionComponent>("sound_dsp_distortion", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::SSoundDspEAXReverbComponent>("sound_dsp_eax_reverb", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::SSoundDspEchoComponent>("sound_dsp_echo", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::SSoundDspEqualizerComponent>("sound_dsp_equalizer", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::SSoundDspFlangerComponent>("sound_dsp_flanger", {"audio/dsp"});
	componentManager.RegisterComponentType<pragma::SCameraComponent>("camera", {"rendering/camera"});
	componentManager.RegisterComponentType<pragma::SDecalComponent>("decal", {"rendering"});
	componentManager.RegisterComponentType<pragma::SExplosionComponent>("explosion", {"gameplay/effects"});
	componentManager.RegisterComponentType<pragma::SFireComponent>("fire", {"gameplay/effects"});
	componentManager.RegisterComponentType<pragma::SFogControllerComponent>("fog_controller", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::SLightComponent>("light", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::SLightDirectionalComponent>("light_directional", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::SLightPointComponent>("light_point", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::SLightSpotComponent>("light_spot", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::SLightSpotVolComponent>("light_spot_volume", {"rendering/lighting"});
	componentManager.RegisterComponentType<pragma::SMicrophoneComponent>("microphone", {"audio"});
	componentManager.RegisterComponentType<pragma::SParticleSystemComponent>("particle_system", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::SQuakeComponent>("quake", {"gameplay/effects"});
	componentManager.RegisterComponentType<pragma::SSmokeTrailComponent>("smoke_trail", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::SSoundComponent>("sound", {"audio"});
	componentManager.RegisterComponentType<pragma::SSoundScapeComponent>("sound_scape", {"audio"});
	componentManager.RegisterComponentType<pragma::SSpriteComponent>("sprite", {"rendering/effects"});
	componentManager.RegisterComponentType<pragma::SEnvTimescaleComponent>("env_time_scale", {"world"});
	componentManager.RegisterComponentType<pragma::SWindComponent>("wind", {"world"});
	componentManager.RegisterComponentType<pragma::SFilterClassComponent>("filter_class", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::SFilterNameComponent>("filter_name", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::SBrushComponent>("brush", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::SKinematicComponent>("kinematic", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::SFuncPhysicsComponent>("func_physics", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::SFuncSoftPhysicsComponent>("func_soft_physics", {"physics", hideInEditor});
	// componentManager.RegisterComponentType<pragma::SFuncPortalComponent>("func_portal", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::SButtonComponent>("button", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::SPlayerSpawnComponent>("player_spawn", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SLogicRelayComponent>("relay", {"logic", hideInEditor});
	componentManager.RegisterComponentType<pragma::SBotComponent>("bot", {"ai", hideInEditor});
	componentManager.RegisterComponentType<pragma::SPointConstraintBallSocketComponent>("constraint_ball_socket", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::SPointConstraintConeTwistComponent>("constraint_cone_twist", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::SPointConstraintDoFComponent>("constraint_dof", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::SPointConstraintFixedComponent>("constraint_fixed", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::SPointConstraintHingeComponent>("constraint_hinge", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::SPointConstraintSliderComponent>("constraint_slider", {"physics/constraints"});
	componentManager.RegisterComponentType<pragma::SPathNodeComponent>("path_node", {"ai", hideInEditor});
	// componentManager.RegisterComponentType<pragma::SRenderTargetComponent>("render_target", {"rendering"});
	componentManager.RegisterComponentType<pragma::SPointTargetComponent>("target", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::PropComponent>("prop", {"physics"});
	componentManager.RegisterComponentType<pragma::SPropDynamicComponent>("prop_dynamic", {"physics"});
	componentManager.RegisterComponentType<pragma::SPropPhysicsComponent>("prop_physics", {"physics"});
	componentManager.RegisterComponentType<pragma::STriggerGravityComponent>("trigger_gravity", {"gameplay/triggers"});
	componentManager.RegisterComponentType<pragma::STriggerHurtComponent>("trigger_hurt", {"gameplay/triggers"});
	componentManager.RegisterComponentType<pragma::STriggerPushComponent>("trigger_push", {"gameplay/triggers"});
	componentManager.RegisterComponentType<pragma::STriggerRemoveComponent>("trigger_remove", {"gameplay/triggers"});
	componentManager.RegisterComponentType<pragma::STriggerTeleportComponent>("trigger_teleport", {"gameplay/triggers"});
	componentManager.RegisterComponentType<pragma::STouchComponent>("touch", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SSkyboxComponent>("skybox", {"world"});
	componentManager.RegisterComponentType<pragma::SWorldComponent>("world", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::SFlashlightComponent>("flashlight", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SShooterComponent>("shooter", {"gameplay"});
	componentManager.RegisterComponentType<pragma::SModelComponent>("model", {"rendering/model"});
	componentManager.RegisterComponentType<pragma::SAnimatedComponent>("animated", {"animation"});
	componentManager.RegisterComponentType<pragma::SGenericComponent>("entity", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::SIOComponent>("io", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::STimeScaleComponent>("time_scale", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::SInfoLandmarkComponent>("info_landmark", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::SAttachmentComponent>("attachment", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::SChildComponent>("child", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::SOwnableComponent>("ownable", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::SDebugTextComponent>("debug_text", {"debug"});
	componentManager.RegisterComponentType<pragma::SDebugPointComponent>("debug_point", {"debug"});
	componentManager.RegisterComponentType<pragma::SDebugLineComponent>("debug_line", {"debug"});
	componentManager.RegisterComponentType<pragma::SDebugBoxComponent>("debug_box", {"debug"});
	componentManager.RegisterComponentType<pragma::SDebugSphereComponent>("debug_sphere", {"debug"});
	componentManager.RegisterComponentType<pragma::SDebugConeComponent>("debug_cone", {"debug"});
	componentManager.RegisterComponentType<pragma::SDebugCylinderComponent>("debug_cylinder", {"debug"});
	componentManager.RegisterComponentType<pragma::SDebugPlaneComponent>("debug_plane", {"debug"});
	componentManager.RegisterComponentType<pragma::SPointAtTargetComponent>("point_at_target", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SGamemodeComponent>("gamemode", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::SGameComponent>("game", {"gameplay", hideInEditor});

	componentManager.RegisterComponentType<pragma::SLiquidComponent>("liquid", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::SBuoyancyComponent>("buoyancy", {"physics"});
	componentManager.RegisterComponentType<pragma::SLiquidSurfaceComponent>("liquid_surface", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::SLiquidVolumeComponent>("liquid_volume", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::SLiquidControlComponent>("liquid_control", {"physics/fluid", hideInEditor});
	componentManager.RegisterComponentType<pragma::SLiquidSurfaceSimulationComponent>("liquid_surface_simulation", {"physics/fluid", hideInEditor});
	// --template-component-register-location
}

pragma::BaseEntityComponent *SGame::CreateLuaEntityComponent(BaseEntity &ent, std::string classname) { return Game::CreateLuaEntityComponent<pragma::SLuaBaseEntityComponent, pragma::lua::SLuaBaseEntityComponentHolder>(ent, classname); }
