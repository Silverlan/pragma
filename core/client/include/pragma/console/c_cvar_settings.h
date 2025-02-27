/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_CVAR_SETTINGS_H__
#define __C_CVAR_SETTINGS_H__
#include "pragma/clientdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>

REGISTER_CONVAR_CL(cl_language, udm::Type::String, "", ConVarFlags::Archive, "Game language. If empty, the system language will be used (if available, otherwise it will default to English).");

REGISTER_CONVAR_CL(cl_render_lod_bias, udm::Type::UInt32, "0", ConVarFlags::Archive, "Model LOD-bias. Higher values means higher LOD-Models will be used at lower distance (=Lower quality).");
REGISTER_CONVAR_CL(cl_render_shadow_lod_bias, udm::Type::UInt32, "10", ConVarFlags::Archive, "Model LOD-bias for shadows. Higher values means higher LOD-Models will be used at lower distance (=Lower quality). This value is added on top of cl_render_lod_bias!");
REGISTER_CONVAR_CL(cl_render_vsync_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Turns vertical sync on or off.");
REGISTER_CONVAR_CL(cl_window_resolution, udm::Type::String, "1280x1024", ConVarFlags::Archive, "Changes the window resolution.");
REGISTER_CONVAR_CL(cl_render_resolution, udm::Type::String, "", ConVarFlags::Archive, "Changes the internal rendering resolution. If left empty, the resolution will be the same as the window resolution!");
REGISTER_CONVAR_CL(cl_render_monitor, udm::Type::UInt32, "0", ConVarFlags::Archive, "Which monitor to use in fullscreen mode.");
REGISTER_CONVAR_CL(cl_gpu_device, udm::Type::String, "", ConVarFlags::Archive, "The unique identifier for the GPU vendor and device to use. If empty (or invalid), the first detected device will be used. Usage: <vendorId,deviceId>");
REGISTER_CONVAR_CL(cl_render_window_mode, udm::Type::UInt8, "1", ConVarFlags::Archive, "0 = Fullscreen, 1 = Windowed, 2 = Windowed no-border.");
REGISTER_CONVAR_CL(cl_render_texture_quality, udm::Type::UInt8, "4", ConVarFlags::Archive, "0 = Minimal, 1 = Low, 2 = Medium, 3 = High, 4 = Very High.");
REGISTER_CONVAR_CL(cl_render_anti_aliasing, udm::Type::UInt8, "2", ConVarFlags::Archive, "0 = No Anti Aliasing, 1 = MSAA, 2 = FXAA.");
REGISTER_CONVAR_CL(cl_render_msaa_samples, udm::Type::UInt8, "1", ConVarFlags::Archive, "1 = MSAAx2, 2 = MSAAx4, 3 = MSAAx8, etc.");
REGISTER_CONVAR_CL(cl_render_fxaa_sub_pixel_aliasing_removal_amount, udm::Type::Float, "0.75", ConVarFlags::Archive, "The amount of sub-pixel aliasing removal.");
REGISTER_CONVAR_CL(cl_render_fxaa_edge_threshold, udm::Type::Float, "0.166", ConVarFlags::Archive, "The minimum amount of local contrast required to apply algorithm.");
REGISTER_CONVAR_CL(cl_render_fxaa_min_edge_threshold, udm::Type::Float, "0.0833", ConVarFlags::Archive, "Trims the algorithm from processing darks.");

REGISTER_CONVAR_CL(cl_render_ssao, udm::Type::Boolean, "0", ConVarFlags::Archive, "1 = Screen space ambient occlusion enabled, 0 = disabled.");
REGISTER_CONVAR_CL(cl_render_fov, udm::Type::Float, "90", ConVarFlags::Archive, "Specifies the horizontal field of view.");
REGISTER_CONVAR_CL(cl_render_brightness, udm::Type::Float, "1", ConVarFlags::Archive, "Scene brightness.");
REGISTER_CONVAR_CL(cl_render_contrast, udm::Type::Float, "1", ConVarFlags::Archive, "Scene contrast.");
REGISTER_CONVAR_CL(cl_render_motion_blur, udm::Type::Float, "0", ConVarFlags::Archive, "Amount of motion blur. 0 = disabled.");
REGISTER_CONVAR_CL(cl_render_texture_filtering, udm::Type::UInt8, "6", ConVarFlags::Archive, "0 = Nearest Neighbour, 1 = Bilinear Filtering, 2 = Trilinear Filtering, 3 = Antisotropic Filtering x2, 4 = Antisotropic Filtering x4, 5 = Antisotropic Filtering x8, 6 = Antisotropic Filtering x16");
REGISTER_CONVAR_CL(cl_render_occlusion_culling, udm::Type::UInt8, "4", ConVarFlags::Archive, "0 = Off, 1 = Brute-Force, 2 = CHC++, 3 = Octree, 4 = BSP +Octree");
REGISTER_CONVAR_CL(cl_render_frustum_culling_enabled, udm::Type::Boolean, "1", ConVarFlags::None, "Enables or disables frustum culling.");
REGISTER_CONVAR_CL(cl_render_depth_of_field, udm::Type::Float, "0", ConVarFlags::Archive, "Depth of field scale.");
REGISTER_CONVAR_CL(cl_render_reflection_quality, udm::Type::UInt8, "2", ConVarFlags::Archive, "Changes the quality of reflections. 0 = Only static reflections, 1 = Dynamic reflections, 2 = Reflections with particle effects.");
REGISTER_CONVAR_CL(cl_render_tone_mapping, udm::Type::Int8, "-1", ConVarFlags::Archive, "Changes the tone-mapping algorithm to use. -1 = Use the default, 0 = Gamma correction only, 1 = Reinhard, 2 = Hejil-Richard, 3 = Uncharted, 4 = Aces, 5 = Gran Turismo");

REGISTER_CONVAR_CL(cl_render_preset, udm::Type::Int8, "-1", ConVarFlags::Archive, "Selected preset in display options.");
REGISTER_CONVAR_CL(cl_render_shadow_resolution, udm::Type::UInt32, "1024", ConVarFlags::Archive, "Shadowmap Resolution. Higher resolution equals higher quality shadows, but is also more expensive to render.");
REGISTER_CONVAR_CL(render_shadow_quality, udm::Type::UInt8, "1", ConVarFlags::Archive, "Shadowmap Quality. This affects the detail of the object shadows that are being rendered");
REGISTER_CONVAR_CL(cl_render_shadow_dynamic, udm::Type::Boolean, "1", ConVarFlags::Archive, "Turns dynamic shadows on or off.");
REGISTER_CONVAR_CL(cl_render_shadow_update_frequency, udm::Type::UInt8, "0", ConVarFlags::Archive, "Update frequency in frames. 0 = Updates every frame, 1 = Updates every second frame, etc.");
REGISTER_CONVAR_CL(cl_render_shadow_pssm_update_frequency_offset, udm::Type::Float, "0", ConVarFlags::Archive, "Update frequency for PSSM shadows in frames, relative to 'cl_render_shadow_update_frequency'.");
REGISTER_CONVAR_CL(cl_render_shadow_pssm_split_count, udm::Type::UInt8, "3", ConVarFlags::Archive, "The number of cascades to be used for PSSM. Cannot be 0 or higher than 4.");

REGISTER_CONVAR_CL(cl_render_shader_quality, udm::Type::UInt8, "8", ConVarFlags::Archive, "Shader quality. The actual effect depends on the shader. 1 = Lowest Quality, 10 = Highest Quality");
REGISTER_CONVAR_CL(cl_render_particle_quality, udm::Type::UInt8, "3", ConVarFlags::Archive, "Quality of particle systems. 0 = No particles will be rendered, 1 = Particles will be unlit, 2 = Particles will receive lighting, 3 = Particles will cast shadows.");
REGISTER_CONVAR_CL(cl_render_present_mode, udm::Type::UInt8, "1", ConVarFlags::Archive, "0 = Immediate, 1 = FIFO, 2 = Mailbox");
REGISTER_CONVAR_CL(cl_material_streaming_enabled, udm::Type::Boolean, "0", ConVarFlags::Archive, "0 = All materials and textures will be loaded immediately (= Slower load times), 1 = All materials and textures will be loaded over time. (= Black textures until loaded)")

REGISTER_CONVAR_CL(cl_flex_phoneme_drag, udm::Type::Float, "0.02", ConVarFlags::Archive, "Changes the time required for facial phoneme flexes to adapt to new phonemes.");

REGISTER_CONVAR_CL(cl_audio_master_volume, udm::Type::Float, "1", ConVarFlags::Archive, "Volume scale for all sounds.");
REGISTER_CONVAR_CL(cl_audio_hrtf_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enables or disables Head-related transfer function.");
REGISTER_CONVAR_CL(cl_audio_streaming_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "0 = All sounds will be loaded immediately (= Slower load times), 1 = Some sounds will be loaded over time. (= Sounds might start with a delay)");
REGISTER_CONVAR_CL(cl_audio_always_play, udm::Type::Boolean, "1", ConVarFlags::Archive, "0 = Don't play sounds if window isn't focused., 1 = Always play sounds");

REGISTER_CONVAR_CL(cl_effects_volume, udm::Type::Float, "1", ConVarFlags::Archive, "Volume scale for effect sounds (e.g. footsteps, gunshots, explosions, etc.).");
REGISTER_CONVAR_CL(cl_music_volume, udm::Type::Float, "1", ConVarFlags::Archive, "Volume scale for music.");
REGISTER_CONVAR_CL(cl_voice_volume, udm::Type::Float, "1", ConVarFlags::Archive, "Volume scale for voice (Coming from players and NPCs).");
REGISTER_CONVAR_CL(cl_gui_volume, udm::Type::Float, "1", ConVarFlags::Archive, "Volume scale for sounds emitted by GUI elements.");

REGISTER_CONVAR_CL(cl_mouse_sensitivity, udm::Type::Float, "0.4", ConVarFlags::Archive, "Scale for the mouse sensitivity.");
REGISTER_CONVAR_CL(cl_mouse_acceleration, udm::Type::Float, "1", ConVarFlags::Archive, "Amount of mouse acceleration. 1 = No acceleration.");
REGISTER_CONVAR_CL(cl_mouse_yaw, udm::Type::Float, "1", ConVarFlags::Archive, "Mouse sensitivity on the yaw axis.");
REGISTER_CONVAR_CL(cl_mouse_pitch, udm::Type::Float, "-1", ConVarFlags::Archive, "Mouse sensitivity on the pitch axis. Use positive values to invert the axis.");

REGISTER_CONVAR_CL(cl_physics_simulation_enabled, udm::Type::Boolean, "1", ConVarFlags::Cheat, "Enables or disables physics simulation.");

REGISTER_CONVAR_CL(cl_render_vr_enabled, udm::Type::Boolean, "0", ConVarFlags::Archive, "Turns support for virtual reality on or off.");
// Obsolete; Resolution is now determined by openvr
//REGISTER_CONVAR_CL(cl_render_vr_resolution,"256x256",ConVarFlags::Archive,"Changes the resolution used when rendering the scene for virtual reality.");
REGISTER_CONVAR_CL(cl_vr_hmd_view_enabled, udm::Type::Boolean, "0", ConVarFlags::Archive, "Enables or disables the view for virtual reality (Only works if virtual reality has been enabled in the menu.).");
REGISTER_CONVAR_CL(cl_vr_mirror_window_enabled, udm::Type::Boolean, "0", ConVarFlags::Archive, "Enables or disables the mirror window for virtual reality (Only works if virtual reality has been enabled in the menu.).");

REGISTER_CONVAR_CL(cl_gpu_timer_queries_enabled, udm::Type::Boolean, "0", static_cast<ConVarFlags>(0), "Enables or disables GPU timer queries for debugging.");

// Water
REGISTER_CONVAR_CL(cl_water_surface_simulation_spacing, udm::Type::Float, "50", ConVarFlags::Archive, "Grid spacing for simulating water surfaces. Lower values will result in a more detailed simulation, but at a great performance cost.");
REGISTER_CONVAR_CL(cl_water_surface_simulation_enable_gpu_acceleration, udm::Type::Boolean, "1", ConVarFlags::Archive, "If enabled, water surfaces will be simulated on the GPU instead of the CPU. This should greatly improve performance in most cases.");
REGISTER_CONVAR_CL(cl_water_surface_simulation_edge_iteration_count, udm::Type::UInt32, "5", ConVarFlags::Archive, "The more iterations, the more detailed the water simulation will be, but at a great performance cost.");

// Controllers
REGISTER_CONVAR_CL(cl_controller_enabled, udm::Type::Boolean, "0", ConVarFlags::Archive, "Enables or disables game controllers / joysticks.");
REGISTER_CONVAR_CL(cl_controller_axis_input_threshold, udm::Type::Float, "0.001", ConVarFlags::Archive, "Axis inputs below this threshold will be ignored for regular console commands.");

// Steam audio
REGISTER_CONVAR_CL(cl_steam_audio_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enables or disables steam audio.");
REGISTER_CONVAR_CL(cl_steam_audio_propagation_delay_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enables or disables propagation delay if steam audio is enabled.");
REGISTER_CONVAR_CL(cl_steam_audio_number_of_rays, udm::Type::UInt32, "16384", ConVarFlags::Archive, "The number of rays to trace from the listener. Range: [1024,131072].");
REGISTER_CONVAR_CL(cl_steam_audio_number_of_diffuse_samples, udm::Type::UInt32, "1024", ConVarFlags::Archive, "The number of directions to consider when a ray bounces off a diffuse (or partly diffuse) surface. Range: [32,4096].");
REGISTER_CONVAR_CL(cl_steam_audio_number_of_bounces, udm::Type::UInt8, "32", ConVarFlags::Archive, "The maximum number of times any ray can bounce within the scene. Range: [1,32].");
REGISTER_CONVAR_CL(cl_steam_audio_ir_duration, udm::Type::Float, "4.0", ConVarFlags::Archive, "The time delay between a sound being emitted and the last audible reflection. Range: [0.5,4.0].");
REGISTER_CONVAR_CL(cl_steam_audio_ambisonics_order, udm::Type::UInt8, "3", ConVarFlags::Archive, "The amount of directional detail in the simulation results. Range: [0,3].");

REGISTER_CONVAR_CL(cl_steam_audio_spatialize_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enables the spatialize DSP effect.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_direct_binaural, udm::Type::Boolean, "1", ConVarFlags::Archive, "Spatialize direct sound using HRTF.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_hrtf_interpolation, udm::Type::UInt8, "1", ConVarFlags::Archive, "HRTF interpolation. 0 = Nearest, 1 = Bilinear.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_distance_attenuation, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enable distance attenuation. If disabled, regular distance attenuation will be used instead.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_air_absorption, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enable air absorption.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_occlusion_mode, udm::Type::UInt8, "1", ConVarFlags::Archive, "Direct occlusion and transmission mode. 0 = Off, 1 = On, No Transmission, 2 = On, Frequency Independent Transmission, 3 = On, Frequency Dependent Transmission.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_occlusion_method, udm::Type::UInt8, "1", ConVarFlags::Archive, "Direct occlusion algorithm. 0 = Raycast, 1 = Partial.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_direct_level, udm::Type::Float, "1.0", ConVarFlags::Archive, "Relative level of direct sound. Range: [0,1].");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_indirect, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enable indirect sound.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_indirect_binaural, udm::Type::Boolean, "1", ConVarFlags::Archive, "Spatialize indirect sound using HRTF.");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_indirect_level, udm::Type::Float, "1.0", ConVarFlags::Archive, "Relative level of indirect sound. Range: [0,10].");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_simulation_type, udm::Type::UInt8, "0", ConVarFlags::Archive, "Real-time or baked. 0 = Real-time, 1 = Baked");
REGISTER_CONVAR_CL(cl_steam_audio_spatialize_static_listener, udm::Type::Boolean, "0", ConVarFlags::Archive, "Uses static listener.");

REGISTER_CONVAR_CL(cl_steam_audio_reverb_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enables the reverb DSP effect.");
REGISTER_CONVAR_CL(cl_steam_audio_reverb_indirect_binaural, udm::Type::Boolean, "1", ConVarFlags::Archive, "Spatialize reverb using HRTF.");
REGISTER_CONVAR_CL(cl_steam_audio_reverb_simulation_type, udm::Type::UInt8, "1", ConVarFlags::Archive, "Real-time or baked. 0 = Real-time, 1 = Baked.");

#endif
