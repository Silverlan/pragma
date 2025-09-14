// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/c_ent_debug.hpp"
#include "pragma/entities/c_world.h"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_entity_character_component_model.hpp"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/entities/components/c_game_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/hitbox_mesh_bvh_builder.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/rasterization/hdr_data.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"

import pragma.client.entities;
import pragma.client.entities.components;

void register_entites()
{
	client_entities::register_entity<CBot>("bot");
	client_entities::register_entity<CWaterSurface>("c_water_surface");
	client_entities::register_entity<CEntDebugText>("debug_text");
	client_entities::register_entity<CEntDebugPoint>("debug_point");
	client_entities::register_entity<CEntDebugLine>("debug_line");
	client_entities::register_entity<CEntDebugBox>("debug_box");
	client_entities::register_entity<CEntDebugSphere>("debug_sphere");
	client_entities::register_entity<CEntDebugCone>("debug_cone");
	client_entities::register_entity<CEntDebugCylinder>("debug_cylinder");
	client_entities::register_entity<CEntDebugPlane>("debug_plane");
	client_entities::register_entity<CBaseEntity>("entity");
	client_entities::register_entity<CEnvCamera>("env_camera");
	client_entities::register_entity<CEnvDecal>("env_decal");
	client_entities::register_entity<CEnvExplosion>("env_explosion");
	client_entities::register_entity<CEnvFire>("env_fire");
	client_entities::register_entity<CEnvFogController>("env_fog_controller");
	client_entities::register_entity<CEnvLightDirectional>("env_light_environment");
	client_entities::register_entity<CEnvLightPoint>("env_light_point");
	client_entities::register_entity<CEnvLightSpot>("env_light_spot");
	client_entities::register_entity<CEnvLightSpotVol>("env_light_spot_vol");
	client_entities::register_entity<CEnvMicrophone>("env_microphone");
	client_entities::register_entity<CEnvParticleSystem>("env_particle_system");
	client_entities::register_entity<CEnvQuake>("env_quake");
	client_entities::register_entity<CEnvReflectionProbe>("env_reflection_probe");
	client_entities::register_entity<CEnvSmokeTrail>("env_smoke_trail");
	client_entities::register_entity<CEnvSound>("env_sound");
	client_entities::register_entity<CEnvSoundDsp>("env_sound_dsp");
	client_entities::register_entity<CEnvSoundDspChorus>("env_sound_dsp_chorus");
	client_entities::register_entity<CEnvSoundDspDistortion>("env_sound_dsp_distortion");
	client_entities::register_entity<CEnvSoundDspEAXReverb>("env_sound_dsp_eaxreverb");
	client_entities::register_entity<CEnvSoundDspEcho>("env_sound_dsp_echo");
	client_entities::register_entity<CEnvSoundDspEqualizer>("env_sound_dsp_equalizer");
	client_entities::register_entity<CEnvSoundDspFlanger>("env_sound_dsp_flanger");
	client_entities::register_entity<CEnvSoundProbe>("env_sound_probe");
	client_entities::register_entity<CEnvSoundScape>("env_soundscape");
	client_entities::register_entity<CEnvSprite>("env_sprite");
	client_entities::register_entity<CEnvTimescale>("env_timescale");
	client_entities::register_entity<CEnvWeather>("env_weather");
	client_entities::register_entity<CEnvWind>("env_wind");
	client_entities::register_entity<CFlashlight>("flashlight");
	client_entities::register_entity<CFilterEntityClass>("filter_entity_class");
	client_entities::register_entity<CFilterEntityName>("filter_entity_name");
	client_entities::register_entity<CFuncBrush>("func_brush");
	client_entities::register_entity<CFuncButton>("func_button");
	client_entities::register_entity<CFuncKinematic>("func_kinematic");
	client_entities::register_entity<CFuncPhysics>("func_physics");
	client_entities::register_entity<CFuncSoftPhysics>("func_physics_softbody");
	client_entities::register_entity<CFuncPortal>("func_portal");
	client_entities::register_entity<CFuncWater>("func_water");
	client_entities::register_entity<CGameEntity>("game");
	client_entities::register_entity<COcclusionCuller>("game_occlusion_culler");
	client_entities::register_entity<CShadowManager>("game_shadow_manager");
	client_entities::register_entity<CGamemode>("gamemode");
	client_entities::register_entity<CListener>("listener");
	client_entities::register_entity<CPlayer>("player");
	client_entities::register_entity<CPointConstraintBallSocket>("point_constraint_ballsocket");
	client_entities::register_entity<CPointConstraintConeTwist>("point_constraint_conetwist");
	client_entities::register_entity<CPointConstraintDoF>("point_constraint_dof");
	client_entities::register_entity<CPointConstraintFixed>("point_constraint_fixed");
	client_entities::register_entity<CPointConstraintHinge>("point_constraint_hinge");
	client_entities::register_entity<CPointConstraintSlider>("point_constraint_slider");
	client_entities::register_entity<CPointRenderTarget>("point_rendertarget");
	client_entities::register_entity<CPointTarget>("point_target");
	client_entities::register_entity<CPropDynamic>("prop_dynamic");
	client_entities::register_entity<CPropPhysics>("prop_physics");
	client_entities::register_entity<CRasterizationRenderer>("rasterization_renderer");
	client_entities::register_entity<CRaytracingRenderer>("raytracing_renderer");
	client_entities::register_entity<CScene>("scene");
	client_entities::register_entity<CSkyCamera>("sky_camera");
	client_entities::register_entity<CSkybox>("skybox");
	client_entities::register_entity<CTriggerTouch>("trigger_touch");
	client_entities::register_entity<CUtilPBRConverter>("util_pbr_converter");
	client_entities::register_entity<CWheel>("vhc_wheel");
	client_entities::register_entity<CViewBody>("viewbody");
	client_entities::register_entity<CViewModel>("viewmodel");
	client_entities::register_entity<CWorld>("world");

    // Register networked entities
    // Note: These have to match the serverside registrations in the exact same order!
    client_entities::register_networked_entity<CWorld>();
	client_entities::register_networked_entity<CPropPhysics>();
	client_entities::register_networked_entity<CEnvParticleSystem>();
	client_entities::register_networked_entity<CEnvSound>();
	client_entities::register_networked_entity<CEnvLightSpot>();
	client_entities::register_networked_entity<CEnvLightDirectional>();
	client_entities::register_networked_entity<CEnvLightPoint>();
	client_entities::register_networked_entity<CSkybox>();
	client_entities::register_networked_entity<CFlashlight>();
	client_entities::register_networked_entity<CEnvCamera>();
	client_entities::register_networked_entity<CFuncButton>();
	client_entities::register_networked_entity<CEnvExplosion>();
	client_entities::register_networked_entity<CEnvQuake>();
	client_entities::register_networked_entity<CEnvSoundDsp>();
	client_entities::register_networked_entity<CEnvSoundDspEAXReverb>();
	client_entities::register_networked_entity<CEnvSoundDspChorus>();
	client_entities::register_networked_entity<CEnvSoundDspDistortion>();
	client_entities::register_networked_entity<CEnvSoundDspEcho>();
	client_entities::register_networked_entity<CEnvSoundDspFlanger>();
	client_entities::register_networked_entity<CEnvTimescale>();
	client_entities::register_networked_entity<CEnvFogController>();
	client_entities::register_networked_entity<CEnvLightSpotVol>();
	client_entities::register_networked_entity<CPointRenderTarget>();
	client_entities::register_networked_entity<CFuncPortal>();
	client_entities::register_networked_entity<CFuncWater>();
	client_entities::register_networked_entity<CFuncPhysics>();
	client_entities::register_networked_entity<CEnvSoundScape>();
	client_entities::register_networked_entity<CFuncBrush>();
	client_entities::register_networked_entity<CEnvMicrophone>();
	client_entities::register_networked_entity<CFuncKinematic>();
	client_entities::register_networked_entity<CBot>();
	client_entities::register_networked_entity<CPointTarget>();
	client_entities::register_networked_entity<CWheel>();
	client_entities::register_networked_entity<CPropDynamic>();
	client_entities::register_networked_entity<CEnvFire>();
	client_entities::register_networked_entity<CEnvSmokeTrail>();
	client_entities::register_networked_entity<CEnvSprite>();
	client_entities::register_networked_entity<CEnvDecal>();
	client_entities::register_networked_entity<CPlayer>();
	client_entities::register_networked_entity<CEntDebugText>();
	client_entities::register_networked_entity<CEntDebugPoint>();
	client_entities::register_networked_entity<CEntDebugLine>();
	client_entities::register_networked_entity<CEntDebugBox>();
	client_entities::register_networked_entity<CEntDebugSphere>();
	client_entities::register_networked_entity<CEntDebugCone>();
	client_entities::register_networked_entity<CEntDebugCylinder>();
	client_entities::register_networked_entity<CEntDebugPlane>();
	client_entities::register_networked_entity<CGameEntity>();

	client_entities::register_networked_entity<CPointConstraintBallSocket>();
	client_entities::register_networked_entity<CPointConstraintConeTwist>();
	client_entities::register_networked_entity<CPointConstraintSlider>();
	client_entities::register_networked_entity<CPointConstraintHinge>();
	client_entities::register_networked_entity<CPointConstraintDoF>();
	client_entities::register_networked_entity<CPointConstraintFixed>();
	client_entities::register_networked_entity<CFuncSoftPhysics>();
}
