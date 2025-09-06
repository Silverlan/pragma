// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/entities/player.h"
#include "pragma/entities/s_npc_dragonworm.h"
#include "pragma/entities/s_ent_debug.hpp"

import pragma.server.entities.components;
import pragma.server.entities.registration;
import pragma.server.server_state;

void register_entites()
{
    server_entities::register_entity<Bot>("bot");
    server_entities::register_entity<EntDebugBox>("debug_box");
    server_entities::register_entity<EntDebugCone>("debug_cone");
    server_entities::register_entity<EntDebugCylinder>("debug_cylinder");
    server_entities::register_entity<EntDebugLine>("debug_line");
    server_entities::register_entity<EntDebugText>("debug_text");
    server_entities::register_entity<EntDebugPlane>("debug_plane");
    server_entities::register_entity<EntDebugPoint>("debug_point");
    server_entities::register_entity<EntDebugSphere>("debug_sphere");
    server_entities::register_entity<SBaseEntity>("entity");
    server_entities::register_entity<EnvCamera>("env_camera");
    server_entities::register_entity<EnvDecal>("env_decal");
    server_entities::register_entity<EnvExplosion>("env_explosion");
    server_entities::register_entity<EnvFire>("env_fire");
    server_entities::register_entity<EnvFogController>("env_fog_controller");
    server_entities::register_entity<EnvLightDirectional>("env_light_environment");
    server_entities::register_entity<EnvLightPoint>("env_light_point");
    server_entities::register_entity<EnvLightSpot>("env_light_spot");
    server_entities::register_entity<EnvLightSpotVol>("env_light_spot_vol");
    server_entities::register_entity<EnvMicrophone>("env_microphone");
    server_entities::register_entity<EnvParticleSystem>("env_particle_system");
    server_entities::register_entity<EnvQuake>("env_quake");
    server_entities::register_entity<EnvSmokeTrail>("env_smoke_trail");
    server_entities::register_entity<EnvSprite>("env_sprite");
    server_entities::register_entity<EnvSound>("env_sound");
    server_entities::register_entity<EnvSoundDsp>("env_sound_dsp");
    server_entities::register_entity<EnvSoundDspChorus>("env_sound_dsp_chorus");
    server_entities::register_entity<EnvSoundDspDistortion>("env_sound_dsp_distortion");
    server_entities::register_entity<EnvSoundDspEAXReverb>("env_sound_dsp_eaxreverb");
    server_entities::register_entity<EnvSoundDspEcho>("env_sound_dsp_echo");
    server_entities::register_entity<EnvSoundDspEqualizer>("env_sound_dsp_equalizer");
    server_entities::register_entity<EnvSoundDspFlanger>("env_sound_dsp_flanger");
    server_entities::register_entity<EnvSoundScape>("env_soundscape");
    server_entities::register_entity<EnvTimescale>("env_timescale");
    server_entities::register_entity<EnvWind>("env_wind");
    server_entities::register_entity<FilterEntityClass>("filter_entity_class");
    server_entities::register_entity<FilterEntityName>("filter_entity_name");
    server_entities::register_entity<Flashlight>("flashlight");
    server_entities::register_entity<FuncKinematic>("func_kinematic");
    server_entities::register_entity<FuncPhysics>("func_physics");
    // server_entities::register_entity<FuncPortal>("func_portal");
    server_entities::register_entity<FuncSoftPhysics>("func_physics_softbody");
    server_entities::register_entity<FuncButton>("func_button");
    server_entities::register_entity<FuncBrush>("func_brush");
    server_entities::register_entity<FuncWater>("func_water");
    server_entities::register_entity<SGameEntity>("game");
    server_entities::register_entity<GamePlayerSpawn>("game_player_spawn");
    server_entities::register_entity<SGamemode>("gamemode");
    server_entities::register_entity<InfoLandmark>("info_landmark");
    server_entities::register_entity<LogicRelay>("logic_relay");
    server_entities::register_entity<NPCDragonWorm>("npc_dragonworm");
    server_entities::register_entity<Player>("player");
    server_entities::register_entity<PointConstraintBallSocket>("point_constraint_ballsocket");
    server_entities::register_entity<PointConstraintConeTwist>("point_constraint_conetwist");
    server_entities::register_entity<PointConstraintDoF>("point_constraint_dof");
    server_entities::register_entity<PointConstraintFixed>("point_constraint_fixed");
    server_entities::register_entity<PointConstraintHinge>("point_constraint_hinge");
    server_entities::register_entity<PointConstraintSlider>("point_constraint_slider");
    server_entities::register_entity<PointPathNode>("point_path_node");
    // server_entities::register_entity<PointRenderTarget>("point_rendertarget");
    server_entities::register_entity<PropDynamic>("prop_dynamic");
    server_entities::register_entity<PropPhysics>("prop_physics");
    server_entities::register_entity<PointTarget>("point_target");
    server_entities::register_entity<Skybox>("skybox");
    server_entities::register_entity<TriggerGravity>("trigger_gravity");
    server_entities::register_entity<TriggerHurt>("trigger_hurt");
    server_entities::register_entity<TriggerPush>("trigger_push");
    server_entities::register_entity<TriggerRemove>("trigger_remove");
    server_entities::register_entity<TriggerTeleport>("trigger_teleport");
    server_entities::register_entity<TriggerTouch>("trigger_touch");
    server_entities::register_entity<SWheel>("vhc_wheel");
    server_entities::register_entity<World>("world");

    // Register networked entities
    // Note: These have to match the clientside registrations in the exact same order!
    server_entities::register_networked_entity<World>();
	server_entities::register_networked_entity<PropPhysics>();
	server_entities::register_networked_entity<EnvParticleSystem>();
	server_entities::register_networked_entity<EnvSound>();
	server_entities::register_networked_entity<EnvLightSpot>();
	server_entities::register_networked_entity<EnvLightDirectional>();
	server_entities::register_networked_entity<EnvLightPoint>();
	server_entities::register_networked_entity<Skybox>();
	server_entities::register_networked_entity<Flashlight>();
	server_entities::register_networked_entity<EnvCamera>();
	server_entities::register_networked_entity<FuncButton>();
	server_entities::register_networked_entity<EnvExplosion>();
	server_entities::register_networked_entity<EnvQuake>();
	server_entities::register_networked_entity<EnvSoundDsp>();
	server_entities::register_networked_entity<EnvSoundDspEAXReverb>();
	server_entities::register_networked_entity<EnvSoundDspChorus>();
	server_entities::register_networked_entity<EnvSoundDspDistortion>();
	server_entities::register_networked_entity<EnvSoundDspEcho>();
	server_entities::register_networked_entity<EnvSoundDspFlanger>();
	server_entities::register_networked_entity<EnvTimescale>();
	server_entities::register_networked_entity<EnvFogController>();
	server_entities::register_networked_entity<EnvLightSpotVol>();
	server_entities::register_networked_entity<PointRenderTarget>();
	server_entities::register_networked_entity<FuncPortal>();
	server_entities::register_networked_entity<FuncWater>();
	server_entities::register_networked_entity<FuncPhysics>();
	server_entities::register_networked_entity<EnvSoundScape>();
	server_entities::register_networked_entity<FuncBrush>();
	server_entities::register_networked_entity<EnvMicrophone>();
	server_entities::register_networked_entity<FuncKinematic>();
	server_entities::register_networked_entity<Bot>();
	server_entities::register_networked_entity<PointTarget>();
	server_entities::register_networked_entity<SWheel>();
	server_entities::register_networked_entity<PropDynamic>();
	server_entities::register_networked_entity<EnvFire>();
	server_entities::register_networked_entity<EnvSmokeTrail>();
	server_entities::register_networked_entity<EnvSprite>();
	server_entities::register_networked_entity<EnvDecal>();
	server_entities::register_networked_entity<Player>();
	server_entities::register_networked_entity<EntDebugText>();
	server_entities::register_networked_entity<EntDebugPoint>();
	server_entities::register_networked_entity<EntDebugLine>();
	server_entities::register_networked_entity<EntDebugBox>();
	server_entities::register_networked_entity<EntDebugSphere>();
	server_entities::register_networked_entity<EntDebugCone>();
	server_entities::register_networked_entity<EntDebugCylinder>();
	server_entities::register_networked_entity<EntDebugPlane>();
	server_entities::register_networked_entity<SGameEntity>();

	server_entities::register_networked_entity<PointConstraintBallSocket>();
	server_entities::register_networked_entity<PointConstraintConeTwist>();
	server_entities::register_networked_entity<PointConstraintSlider>();
	server_entities::register_networked_entity<PointConstraintHinge>();
	server_entities::register_networked_entity<PointConstraintDoF>();
	server_entities::register_networked_entity<PointConstraintFixed>();
	server_entities::register_networked_entity<FuncSoftPhysics>();
}
