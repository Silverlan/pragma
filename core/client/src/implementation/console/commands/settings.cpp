// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.register_commands;

namespace {
	using namespace pragma::console::client;
	auto UVN = register_variable<udm::Boolean>("cl_allowdownload", true, pragma::console::ConVarFlags::Archive, "");

	auto UVN = register_variable<udm::String>("cl_language", "", pragma::console::ConVarFlags::Archive, "Game language. If empty, the system language will be used (if available, otherwise it will default to English).");

	auto UVN = register_variable<udm::UInt32>("cl_render_lod_bias", 0, pragma::console::ConVarFlags::Archive, "Model LOD-bias. Higher values means higher LOD-Models will be used at lower distance (=Lower quality).");
	auto UVN = register_variable<udm::UInt32>("cl_render_shadow_lod_bias", 10, pragma::console::ConVarFlags::Archive, "Model LOD-bias for shadows. Higher values means higher LOD-Models will be used at lower distance (=Lower quality). This value is added on top of cl_render_lod_bias!");
	auto UVN = register_variable<udm::Boolean>("cl_render_vsync_enabled", true, pragma::console::ConVarFlags::Archive, "Turns vertical sync on or off.");
	auto UVN = register_variable<udm::String>("cl_window_resolution", "1280x1024", pragma::console::ConVarFlags::Archive, "Changes the window resolution.");
	auto UVN = register_variable<udm::String>("cl_render_resolution", "", pragma::console::ConVarFlags::Archive, "Changes the internal rendering resolution. If left empty, the resolution will be the same as the window resolution!");
	auto UVN = register_variable<udm::UInt32>("cl_render_monitor", 0, pragma::console::ConVarFlags::Archive, "Which monitor to use in fullscreen mode.");
	auto UVN = register_variable<udm::String>("cl_gpu_device", "", pragma::console::ConVarFlags::Archive, "The unique identifier for the GPU vendor and device to use. If empty (or invalid), the first detected device will be used. Usage: <vendorId,deviceId>");
	auto UVN = register_variable<udm::UInt8>("cl_render_window_mode", 1, pragma::console::ConVarFlags::Archive, "0 = Fullscreen, 1 = Windowed, 2 = Windowed no-border.");
	auto UVN = register_variable<udm::UInt8>("cl_render_texture_quality", 4, pragma::console::ConVarFlags::Archive, "0 = Minimal, 1 = Low, 2 = Medium, 3 = High, 4 = Very High.");
	auto UVN = register_variable<udm::UInt8>("cl_render_anti_aliasing", 2, pragma::console::ConVarFlags::Archive, "0 = No Anti Aliasing, 1 = MSAA, 2 = FXAA.");
	auto UVN = register_variable<udm::UInt8>("cl_render_msaa_samples", 1, pragma::console::ConVarFlags::Archive, "1 = MSAAx2, 2 = MSAAx4, 3 = MSAAx8, etc.");
	auto UVN = register_variable<udm::Float>("cl_render_fxaa_sub_pixel_aliasing_removal_amount", 0.75, pragma::console::ConVarFlags::Archive, "The amount of sub-pixel aliasing removal.");
	auto UVN = register_variable<udm::Float>("cl_render_fxaa_edge_threshold", 0.166, pragma::console::ConVarFlags::Archive, "The minimum amount of local contrast required to apply algorithm.");
	auto UVN = register_variable<udm::Float>("cl_render_fxaa_min_edge_threshold", 0.0833, pragma::console::ConVarFlags::Archive, "Trims the algorithm from processing darks.");

	auto UVN = register_variable<udm::Boolean>("cl_render_ssao", false, pragma::console::ConVarFlags::Archive, "1 = Screen space ambient occlusion enabled, 0 = disabled.");
	auto UVN = register_variable<udm::Float>("cl_render_fov", 90, pragma::console::ConVarFlags::Archive, "Specifies the horizontal field of view.");
	auto UVN = register_variable<udm::Float>("cl_render_brightness", 1, pragma::console::ConVarFlags::Archive, "Scene brightness.");
	auto UVN = register_variable<udm::Float>("cl_render_contrast", 1, pragma::console::ConVarFlags::Archive, "Scene contrast.");
	auto UVN = register_variable<udm::Float>("cl_render_motion_blur", 0, pragma::console::ConVarFlags::Archive, "Amount of motion blur. 0 = disabled.");
	auto UVN = register_variable<udm::UInt8>("cl_render_texture_filtering", 6, pragma::console::ConVarFlags::Archive,
	  "0 = Nearest Neighbour, 1 = Bilinear Filtering, 2 = Trilinear Filtering, 3 = Antisotropic Filtering x2, 4 = Antisotropic Filtering x4, 5 = Antisotropic Filtering x8, 6 = Antisotropic Filtering x16");
	auto UVN = register_variable<udm::UInt8>("cl_render_occlusion_culling", 4, pragma::console::ConVarFlags::Archive, "0 = Off, 1 = Brute-Force, 2 = CHC++, 3 = Octree, 4 = BSP +Octree");
	auto UVN = register_variable<udm::Boolean>("cl_render_frustum_culling_enabled", true, pragma::console::ConVarFlags::None, "Enables or disables frustum culling.");
	auto UVN = register_variable<udm::Float>("cl_render_depth_of_field", 0, pragma::console::ConVarFlags::Archive, "Depth of field scale.");
	auto UVN = register_variable<udm::UInt8>("cl_render_reflection_quality", 2, pragma::console::ConVarFlags::Archive, "Changes the quality of reflections. 0 = Only static reflections, 1 = Dynamic reflections, 2 = Reflections with particle effects.");
	auto UVN = register_variable<udm::Int8>("cl_render_tone_mapping", -1, pragma::console::ConVarFlags::Archive, "Changes the tone-mapping algorithm to use. -1 = Use the default, 0 = Gamma correction only, 1 = Reinhard, 2 = Hejil-Richard, 3 = Uncharted, 4 = Aces, 5 = Gran Turismo");

	auto UVN = register_variable<udm::Int8>("cl_render_preset", -1, pragma::console::ConVarFlags::Archive, "Selected preset in display options.");
	auto UVN = register_variable<udm::UInt32>("cl_render_shadow_resolution", 1024, pragma::console::ConVarFlags::Archive, "Shadowmap Resolution. Higher resolution equals higher quality shadows, but is also more expensive to render.");
	auto UVN = register_variable<udm::UInt8>("render_shadow_quality", 1, pragma::console::ConVarFlags::Archive, "Shadowmap Quality. This affects the detail of the object shadows that are being rendered");
	auto UVN = register_variable<udm::Boolean>("cl_render_shadow_dynamic", true, pragma::console::ConVarFlags::Archive, "Turns dynamic shadows on or off.");
	auto UVN = register_variable<udm::UInt8>("cl_render_shadow_update_frequency", 0, pragma::console::ConVarFlags::Archive, "Update frequency in frames. 0 = Updates every frame, 1 = Updates every second frame, etc.");
	auto UVN = register_variable<udm::Float>("cl_render_shadow_pssm_update_frequency_offset", 0, pragma::console::ConVarFlags::Archive, "Update frequency for PSSM shadows in frames, relative to 'cl_render_shadow_update_frequency'.");
	auto UVN = register_variable<udm::UInt8>("cl_render_shadow_pssm_split_count", 3, pragma::console::ConVarFlags::Archive, "The number of cascades to be used for PSSM. Cannot be 0 or higher than 4.");

	auto UVN = register_variable<udm::UInt8>("cl_render_shader_quality", 8, pragma::console::ConVarFlags::Archive, "Shader quality. The actual effect depends on the shader. 1 = Lowest Quality, 10 = Highest Quality");
	auto UVN = register_variable<udm::UInt8>("cl_render_particle_quality", 3, pragma::console::ConVarFlags::Archive, "Quality of particle systems. 0 = No particles will be rendered, 1 = Particles will be unlit, 2 = Particles will receive lighting, 3 = Particles will cast shadows.");
	auto UVN = register_variable<udm::UInt8>("cl_render_present_mode", 1, pragma::console::ConVarFlags::Archive, "0 = Immediate, 1 = FIFO, 2 = Mailbox");
	auto UVN = register_variable<udm::Boolean>("cl_material_streaming_enabled", false, pragma::console::ConVarFlags::Archive,
	  "0 = All materials and textures will be loaded immediately (= Slower load times), 1 = All materials and textures will be loaded over time. (= Black textures until loaded)");

	auto UVN = register_variable<udm::Float>("cl_flex_phoneme_drag", 0.02, pragma::console::ConVarFlags::Archive, "Changes the time required for facial phoneme flexes to adapt to new phonemes.");

	auto UVN = register_variable<udm::Float>("cl_audio_master_volume", 1, pragma::console::ConVarFlags::Archive, "Volume scale for all sounds.");
	auto UVN = register_variable<udm::Boolean>("cl_audio_hrtf_enabled", true, pragma::console::ConVarFlags::Archive, "Enables or disables Head-related transfer function.");
	auto UVN = register_variable<udm::Boolean>("cl_audio_streaming_enabled", true, pragma::console::ConVarFlags::Archive, "0 = All sounds will be loaded immediately (= Slower load times), 1 = Some sounds will be loaded over time. (= Sounds might start with a delay)");
	auto UVN = register_variable<udm::Boolean>("cl_audio_always_play", true, pragma::console::ConVarFlags::Archive, "0 = Don't play sounds if window isn't focused., 1 = Always play sounds");

	auto UVN = register_variable<udm::Float>("cl_effects_volume", 1, pragma::console::ConVarFlags::Archive, "Volume scale for effect sounds (e.g. footsteps, gunshots, explosions, etc.).");
	auto UVN = register_variable<udm::Float>("cl_music_volume", 1, pragma::console::ConVarFlags::Archive, "Volume scale for music.");
	auto UVN = register_variable<udm::Float>("cl_voice_volume", 1, pragma::console::ConVarFlags::Archive, "Volume scale for voice (Coming from players and NPCs).");
	auto UVN = register_variable<udm::Float>("cl_gui_volume", 1, pragma::console::ConVarFlags::Archive, "Volume scale for sounds emitted by GUI elements.");

	auto UVN = register_variable<udm::Float>("cl_mouse_sensitivity", 0.4, pragma::console::ConVarFlags::Archive, "Scale for the mouse sensitivity.");
	auto UVN = register_variable<udm::Float>("cl_mouse_acceleration", 1, pragma::console::ConVarFlags::Archive, "Amount of mouse acceleration. 1 = No acceleration.");
	auto UVN = register_variable<udm::Float>("cl_mouse_yaw", 1, pragma::console::ConVarFlags::Archive, "Mouse sensitivity on the yaw axis.");
	auto UVN = register_variable<udm::Float>("cl_mouse_pitch", -1, pragma::console::ConVarFlags::Archive, "Mouse sensitivity on the pitch axis. Use positive values to invert the axis.");

	auto UVN = register_variable<udm::Boolean>("cl_physics_simulation_enabled", true, pragma::console::ConVarFlags::Cheat, "Enables or disables physics simulation.");

	auto UVN = register_variable<udm::Boolean>("cl_render_vr_enabled", false, pragma::console::ConVarFlags::Archive, "Turns support for virtual reality on or off.");
	auto UVN = register_variable<udm::Boolean>("cl_vr_hmd_view_enabled", false, pragma::console::ConVarFlags::Archive, "Enables or disables the view for virtual reality (Only works if virtual reality has been enabled in the menu.).");
	auto UVN = register_variable<udm::Boolean>("cl_vr_mirror_window_enabled", false, pragma::console::ConVarFlags::Archive, "Enables or disables the mirror window for virtual reality (Only works if virtual reality has been enabled in the menu.).");

	auto UVN = register_variable<udm::Boolean>("cl_gpu_timer_queries_enabled", false, static_cast<pragma::console::ConVarFlags>(0), "Enables or disables GPU timer queries for debugging.");

	// Water
	auto UVN = register_variable<udm::Float>("cl_water_surface_simulation_spacing", 50, pragma::console::ConVarFlags::Archive, "Grid spacing for simulating water surfaces. Lower values will result in a more detailed simulation, but at a great performance cost.");
	auto UVN = register_variable<udm::Boolean>("cl_water_surface_simulation_enable_gpu_acceleration", true, pragma::console::ConVarFlags::Archive, "If enabled, water surfaces will be simulated on the GPU instead of the CPU. This should greatly improve performance in most cases.");
	auto UVN = register_variable<udm::UInt32>("cl_water_surface_simulation_edge_iteration_count", 5, pragma::console::ConVarFlags::Archive, "The more iterations, the more detailed the water simulation will be, but at a great performance cost.");

	// Controllers
	auto UVN = register_variable<udm::Boolean>("cl_controller_enabled", false, pragma::console::ConVarFlags::Archive, "Enables or disables game controllers / joysticks.");
	auto UVN = register_variable<udm::Float>("cl_controller_axis_input_threshold", 0.001, pragma::console::ConVarFlags::Archive, "Axis inputs below this threshold will be ignored for regular console commands.");

	// Steam audio
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_enabled", true, pragma::console::ConVarFlags::Archive, "Enables or disables steam audio.");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_propagation_delay_enabled", true, pragma::console::ConVarFlags::Archive, "Enables or disables propagation delay if steam audio is enabled.");
	auto UVN = register_variable<udm::UInt32>("cl_steam_audio_number_of_rays", 16384, pragma::console::ConVarFlags::Archive, "The number of rays to trace from the listener. Range: [1024,131072].");
	auto UVN = register_variable<udm::UInt32>("cl_steam_audio_number_of_diffuse_samples", 1024, pragma::console::ConVarFlags::Archive, "The number of directions to consider when a ray bounces off a diffuse (or partly diffuse) surface. Range: [32,4096].");
	auto UVN = register_variable<udm::UInt8>("cl_steam_audio_number_of_bounces", 32, pragma::console::ConVarFlags::Archive, "The maximum number of times any ray can bounce within the scene. Range: [1,32].");
	auto UVN = register_variable<udm::Float>("cl_steam_audio_ir_duration", 4.0, pragma::console::ConVarFlags::Archive, "The time delay between a sound being emitted and the last audible reflection. Range: [0.5,4.0].");
	auto UVN = register_variable<udm::UInt8>("cl_steam_audio_ambisonics_order", 3, pragma::console::ConVarFlags::Archive, "The amount of directional detail in the simulation results. Range: [0,3].");

	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_spatialize_enabled", true, pragma::console::ConVarFlags::Archive, "Enables the spatialize DSP effect.");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_spatialize_direct_binaural", true, pragma::console::ConVarFlags::Archive, "Spatialize direct sound using HRTF.");
	auto UVN = register_variable<udm::UInt8>("cl_steam_audio_spatialize_hrtf_interpolation", 1, pragma::console::ConVarFlags::Archive, "HRTF interpolation. 0 = Nearest, 1 = Bilinear.");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_spatialize_distance_attenuation", true, pragma::console::ConVarFlags::Archive, "Enable distance attenuation. If disabled, regular distance attenuation will be used instead.");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_spatialize_air_absorption", true, pragma::console::ConVarFlags::Archive, "Enable air absorption.");
	auto UVN
	  = register_variable<udm::UInt8>("cl_steam_audio_spatialize_occlusion_mode", 1, pragma::console::ConVarFlags::Archive, "Direct occlusion and transmission mode. 0 = Off, 1 = On, No Transmission, 2 = On, Frequency Independent Transmission, 3 = On, Frequency Dependent Transmission.");
	auto UVN = register_variable<udm::UInt8>("cl_steam_audio_spatialize_occlusion_method", 1, pragma::console::ConVarFlags::Archive, "Direct occlusion algorithm. 0 = Raycast, 1 = Partial.");
	auto UVN = register_variable<udm::Float>("cl_steam_audio_spatialize_direct_level", 1.0, pragma::console::ConVarFlags::Archive, "Relative level of direct sound. Range: [0,1].");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_spatialize_indirect", true, pragma::console::ConVarFlags::Archive, "Enable indirect sound.");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_spatialize_indirect_binaural", true, pragma::console::ConVarFlags::Archive, "Spatialize indirect sound using HRTF.");
	auto UVN = register_variable<udm::Float>("cl_steam_audio_spatialize_indirect_level", 1.0, pragma::console::ConVarFlags::Archive, "Relative level of indirect sound. Range: [0,10].");
	auto UVN = register_variable<udm::UInt8>("cl_steam_audio_spatialize_simulation_type", 0, pragma::console::ConVarFlags::Archive, "Real-time or baked. 0 = Real-time, 1 = Baked");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_spatialize_static_listener", false, pragma::console::ConVarFlags::Archive, "Uses static listener.");

	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_reverb_enabled", true, pragma::console::ConVarFlags::Archive, "Enables the reverb DSP effect.");
	auto UVN = register_variable<udm::Boolean>("cl_steam_audio_reverb_indirect_binaural", true, pragma::console::ConVarFlags::Archive, "Spatialize reverb using HRTF.");
	auto UVN = register_variable<udm::UInt8>("cl_steam_audio_reverb_simulation_type", 1, pragma::console::ConVarFlags::Archive, "Real-time or baked. 0 = Real-time, 1 = Baked.");
}
