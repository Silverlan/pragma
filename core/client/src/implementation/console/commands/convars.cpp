// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

namespace {
	using namespace pragma::console::client;
	auto UVN = register_variable<udm::Boolean>("cl_allowdownload", true, ConVarFlags::Archive, "");
	auto UVN = register_variable<udm::Boolean>("cl_allowupload", true, ConVarFlags::Archive, "");
	auto UVN = register_variable<udm::Float>("cl_fov", 75, ConVarFlags::Archive, "Horizontal world field of view.");
	auto UVN = register_variable<udm::Float>("cl_fov_viewmodel", 70, ConVarFlags::Archive, "Horizontal view model field of view.");
	auto UVN = register_variable<udm::Boolean>("debug_nav_show_meshes", false, ConVarFlags::Cheat, "Shows the navigation mesh for this map.");
	auto UVN = register_variable<udm::Boolean>("debug_physics_draw", false, ConVarFlags::Cheat, "Draws various physics information as wireframe.");
	auto UVN = register_variable<udm::Boolean>("sv_debug_physics_draw", false, ConVarFlags::Cheat | ConVarFlags::Singleplayer, "Draws various serverside physics information as wireframe.");
	auto UVN = register_variable<udm::Boolean>("debug_bsp_nodes", false, ConVarFlags::Cheat,
	  "Prints and draws various debug information about the binary space partitioning of the level (if available). 1 = Draw leaf of camera location and all visible leaves, 2 = Same, but doesn't update continuously.");
	auto UVN = register_variable<udm::Boolean>("debug_bsp_lock", false, ConVarFlags::Cheat, "Locks BSP culling so it doesn't change when the camera moves.");
	auto UVN = register_variable<udm::Boolean>("debug_render_octree_static_draw", false, ConVarFlags::Cheat, "Draws the octree for static world geometry.");
	auto UVN = register_variable<udm::Boolean>("debug_render_octree_dynamic_draw", false, ConVarFlags::Cheat, "Draws the octree for dynamic objects.");
	auto UVN = register_variable<udm::Boolean>("debug_ai_navigation", false, ConVarFlags::Cheat, "Displays the current navigation path for all NPCs.");
	auto UVN = register_variable<udm::Boolean>("debug_steam_audio_probe_boxes", false, ConVarFlags::Cheat, "Displays the sound probe spheres for the current map.");
	auto UVN = register_variable<udm::Float>("cl_fps_decay_factor", 0.8, ConVarFlags::None, "How slowly to decay the previous fps.");

	auto UVN = register_variable<udm::Boolean>("debug_particle_blob_show_neighbor_links", false, ConVarFlags::Cheat, "Displays the links to adjacent neighbors for blob particles.");

	auto UVN = register_variable<udm::Boolean>("debug_render_lock_render_queues", false, ConVarFlags::None, "Locks the current render queues and effectively disables the render queue thread.");
	auto UVN = register_variable<udm::Boolean>("debug_render_lock_render_command_buffers", false, ConVarFlags::None, "Locks the current render command buffers and causes them to not be re-created every frame.");
	auto UVN = register_variable<udm::Boolean>("debug_render_normals", false, ConVarFlags::Cheat, "Displays all normals as lines in the world. 0 = Disabled, 1 = Display normals, 2 = Display normals, tangents and bitangents, 3 = Display face normals, 4 = Display tangents, 5 = Display bitangents.");
	auto UVN = register_variable<udm::Boolean>("cl_show_fps", false, ConVarFlags::None, "Displays the current fps at the top left of the screen if enabled.");

	auto UVN = register_variable<udm::Boolean>("render_vsync_enabled", true, ConVarFlags::Archive, "1 = Vertical sync enabled, 0 = disabled.");
	auto UVN = register_variable<udm::Boolean>("render_normalmapping_enabled", true, ConVarFlags::Archive, "1 = Normal mapping enabled, 0 = disabled.");
	auto UVN = register_variable<udm::Float>("render_csm_max_distance", 8192, ConVarFlags::Archive, "The maximum distance at which csm shadows are casted. A larger distance means lower quality.");
	auto UVN = register_variable<udm::UInt8>("render_texture_filtering", 1, ConVarFlags::Archive, "0 = Nearest-Neighbor, 1 = Bilinear, 2 = trilinear.");
	auto UVN = register_variable<udm::Float>("render_hdr_max_exposure", 8, ConVarFlags::Archive, "Expore will be clamped at this value. Can be overwritten by the map.");
	auto UVN = register_variable<udm::Boolean>("render_draw_scene", true, ConVarFlags::Cheat, "1 = Default, 0 = Scene isn't drawn.");
	auto UVN = register_variable<udm::UInt8>("render_draw_world", 1, ConVarFlags::Cheat, "1 = Default, 2 = Wireframe, 0 = Don't draw world.");
	auto UVN = register_variable<udm::Boolean>("render_draw_static", true, ConVarFlags::Cheat, "1 = Default, 0 = Don't draw level geometry.");
	auto UVN = register_variable<udm::Boolean>("render_draw_dynamic", true, ConVarFlags::Cheat, "1 = Default, 0 = Don't draw props.");
	auto UVN = register_variable<udm::Boolean>("render_draw_translucent", true, ConVarFlags::Cheat, "1 = Default, 0 = Don't draw translucent objects.");
	auto UVN = register_variable<udm::Boolean>("render_draw_sky", true, ConVarFlags::Cheat, "1 = Default, 0 = Sky isn't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_water", true, ConVarFlags::Cheat, "1 = Default, 0 = Water isn't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_view", true, ConVarFlags::Cheat, "1 = Default, 0 = View-Models aren't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_particles", true, ConVarFlags::Cheat, "1 = Default, 0 = Particles aren't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_glow", true, ConVarFlags::Cheat, "1 = Default, 0 = Glow-effects aren't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_clear_scene", false, ConVarFlags::None, "0 = Default, 1 = Screen is cleared before scene is rendered.");
	auto UVN = register_variable<udm::Vector4i>("render_clear_scene_color", Vector4i{0, 0, 0, 255}, ConVarFlags::None, "Clear color to use if render_clear_scene is set to 1.");
	auto UVN = register_variable<udm::UInt32>("render_forwardplus_tile_size", 16, ConVarFlags::None, "Changes the tile size used for the light culling in forward+.");
	auto UVN = register_variable<udm::UInt8>("render_technique", 0, ConVarFlags::None, "0 = Rasterization, 1 = Raytracing");
	auto UVN = register_variable<udm::String>("sky_override", "", ConVarFlags::None, "Replaces the material for all current skyboxes with the specified material.");

	auto UVN = register_variable<udm::Boolean>("render_pp_nightvision", false, ConVarFlags::None, "Enables or disables night vision.");
	auto UVN = register_variable<udm::Float>("render_pp_nightvision_luminance_threshold", 0.2, ConVarFlags::Archive, "Changes the luminance threshold for the night vision post-processing effect.");
	auto UVN = register_variable<udm::Float>("render_pp_nightvision_color_amplification", 4, ConVarFlags::Archive, "Amplification factor for pixels with low luminance.");
	auto UVN = register_variable<udm::String>("render_pp_nightvision_noise_texture", "effects/noise1", ConVarFlags::Archive, "Noise texture to use for the night vision post-processing effect.");

	auto UVN = register_variable<udm::Boolean>("render_bloom_enabled", true, ConVarFlags::Archive, "Enables or disables bloom and glow effects.");
	auto UVN = register_variable<udm::UInt32>("render_bloom_amount", 9, ConVarFlags::Archive, "Number of render passes to use for bloom.");
	auto UVN = register_variable<udm::UInt32>("render_bloom_resolution", 512, ConVarFlags::Archive, "The width for the bloom texture. The height will be calculated using the aspect ratio of the renderer.");

	auto UVN = register_variable<udm::Boolean>("debug_occlusion_culling_freeze_camera", false, ConVarFlags::None, "Freezes the current camera position in place for occlusion culling.");
}
