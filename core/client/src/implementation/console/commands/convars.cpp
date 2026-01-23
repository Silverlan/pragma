// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

namespace {
	using namespace pragma::console::client;
	auto UVN = register_variable<udm::Boolean>("cl_allowdownload", true, pragma::console::ConVarFlags::Archive, "");
	auto UVN = register_variable<udm::Boolean>("cl_allowupload", true, pragma::console::ConVarFlags::Archive, "");
	auto UVN = register_variable<udm::Float>("cl_fov", 75, pragma::console::ConVarFlags::Archive, "Horizontal world field of view.");
	auto UVN = register_variable<udm::Float>("cl_fov_viewmodel", 70, pragma::console::ConVarFlags::Archive, "Horizontal view model field of view.");
	auto UVN = register_variable<udm::Boolean>("debug_nav_show_meshes", false, pragma::console::ConVarFlags::Cheat, "Shows the navigation mesh for this map.");
	auto UVN = register_variable<udm::Boolean>("debug_physics_draw", false, pragma::console::ConVarFlags::Cheat, "Draws various physics information as wireframe.");
	auto UVN = register_variable<udm::Boolean>("sv_debug_physics_draw", false, pragma::console::ConVarFlags::Cheat | pragma::console::ConVarFlags::Singleplayer, "Draws various serverside physics information as wireframe.");
	auto UVN = register_variable<udm::Boolean>("debug_bsp_nodes", false, pragma::console::ConVarFlags::Cheat,
	  "Prints and draws various debug information about the binary space partitioning of the level (if available). 1 = Draw leaf of camera location and all visible leaves, 2 = Same, but doesn't update continuously.");
	auto UVN = register_variable<udm::Boolean>("debug_bsp_lock", false, pragma::console::ConVarFlags::Cheat, "Locks BSP culling so it doesn't change when the camera moves.");
	auto UVN = register_variable<udm::Boolean>("debug_render_octree_static_draw", false, pragma::console::ConVarFlags::Cheat, "Draws the octree for static world geometry.");
	auto UVN = register_variable<udm::Boolean>("debug_render_octree_dynamic_draw", false, pragma::console::ConVarFlags::Cheat, "Draws the octree for dynamic objects.");
	auto UVN = register_variable<udm::Boolean>("debug_ai_navigation", false, pragma::console::ConVarFlags::Cheat, "Displays the current navigation path for all NPCs.");
	auto UVN = register_variable<udm::Boolean>("debug_steam_audio_probe_boxes", false, pragma::console::ConVarFlags::Cheat, "Displays the sound probe spheres for the current map.");
	auto UVN = register_variable<udm::Float>("cl_fps_decay_factor", 0.8, pragma::console::ConVarFlags::None, "How slowly to decay the previous fps.");

	auto UVN = register_variable<udm::Boolean>("debug_particle_blob_show_neighbor_links", false, pragma::console::ConVarFlags::Cheat, "Displays the links to adjacent neighbors for blob particles.");

	auto UVN = register_variable<udm::Boolean>("debug_render_lock_render_queues", false, pragma::console::ConVarFlags::None, "Locks the current render queues and effectively disables the render queue thread.");
	auto UVN = register_variable<udm::Boolean>("debug_render_lock_render_command_buffers", false, pragma::console::ConVarFlags::None, "Locks the current render command buffers and causes them to not be re-created every frame.");
	auto UVN = register_variable<udm::Boolean>("debug_render_normals", false, pragma::console::ConVarFlags::Cheat,
	  "Displays all normals as lines in the world. 0 = Disabled, 1 = Display normals, 2 = Display normals, tangents and bitangents, 3 = Display face normals, 4 = Display tangents, 5 = Display bitangents.");
	auto UVN = register_variable<udm::Boolean>("cl_show_fps", false, pragma::console::ConVarFlags::None, "Displays the current fps at the top left of the screen if enabled.");

	auto UVN = register_variable<udm::Boolean>("render_vsync_enabled", true, pragma::console::ConVarFlags::Archive, "1 = Vertical sync enabled, 0 = disabled.");
	auto UVN = register_variable<udm::Boolean>("render_normalmapping_enabled", true, pragma::console::ConVarFlags::Archive, "1 = Normal mapping enabled, 0 = disabled.");
	auto UVN = register_variable<udm::Float>("render_csm_max_distance", 8192, pragma::console::ConVarFlags::Archive, "The maximum distance at which csm shadows are casted. A larger distance means lower quality.");
	auto UVN = register_variable<udm::UInt8>("render_texture_filtering", 1, pragma::console::ConVarFlags::Archive, "0 = Nearest-Neighbor, 1 = Bilinear, 2 = trilinear.");
	auto UVN = register_variable<udm::Float>("render_hdr_max_exposure", 8, pragma::console::ConVarFlags::Archive, "Expore will be clamped at this value. Can be overwritten by the map.");
	auto UVN = register_variable<udm::Boolean>("render_draw_scene", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Scene isn't drawn.");
	auto UVN = register_variable<udm::UInt8>("render_draw_world", 1, pragma::console::ConVarFlags::Cheat, "1 = Default, 2 = Wireframe, 0 = Don't draw world.");
	auto UVN = register_variable<udm::Boolean>("render_draw_static", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Don't draw level geometry.");
	auto UVN = register_variable<udm::Boolean>("render_draw_dynamic", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Don't draw props.");
	auto UVN = register_variable<udm::Boolean>("render_draw_translucent", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Don't draw translucent objects.");
	auto UVN = register_variable<udm::Boolean>("render_draw_sky", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Sky isn't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_water", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Water isn't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_view", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = View-Models aren't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_particles", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Particles aren't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_draw_glow", true, pragma::console::ConVarFlags::Cheat, "1 = Default, 0 = Glow-effects aren't drawn.");
	auto UVN = register_variable<udm::Boolean>("render_clear_scene", false, pragma::console::ConVarFlags::None, "0 = Default, 1 = Screen is cleared before scene is rendered.");
	auto UVN = register_variable<udm::Vector4i>("render_clear_scene_color", Vector4i {0, 0, 0, 255}, pragma::console::ConVarFlags::None, "Clear color to use if render_clear_scene is set to 1.");
	auto UVN = register_variable<udm::UInt32>("render_forwardplus_tile_size", 16, pragma::console::ConVarFlags::None, "Changes the tile size used for the light culling in forward+.");
	auto UVN = register_variable<udm::UInt8>("render_technique", 0, pragma::console::ConVarFlags::None, "0 = Rasterization, 1 = Raytracing");
	auto UVN = register_variable<udm::String>("sky_override", "", pragma::console::ConVarFlags::None, "Replaces the material for all current skyboxes with the specified material.");

	auto UVN = register_variable<udm::Boolean>("render_pp_nightvision", false, pragma::console::ConVarFlags::None, "Enables or disables night vision.");
	auto UVN = register_variable<udm::Float>("render_pp_nightvision_luminance_threshold", 0.2, pragma::console::ConVarFlags::Archive, "Changes the luminance threshold for the night vision post-processing effect.");
	auto UVN = register_variable<udm::Float>("render_pp_nightvision_color_amplification", 4, pragma::console::ConVarFlags::Archive, "Amplification factor for pixels with low luminance.");
	auto UVN = register_variable<udm::String>("render_pp_nightvision_noise_texture", "effects/noise1", pragma::console::ConVarFlags::Archive, "Noise texture to use for the night vision post-processing effect.");

	auto UVN = register_variable<udm::Boolean>("render_bloom_enabled", true, pragma::console::ConVarFlags::Archive, "Enables or disables bloom and glow effects.");
	auto UVN = register_variable<udm::UInt32>("render_bloom_amount", 9, pragma::console::ConVarFlags::Archive, "Number of render passes to use for bloom.");
	auto UVN = register_variable<udm::UInt32>("render_bloom_resolution", 512, pragma::console::ConVarFlags::Archive, "The width for the bloom texture. The height will be calculated using the aspect ratio of the renderer.");

	auto UVN = register_variable<udm::Boolean>("debug_occlusion_culling_freeze_camera", false, pragma::console::ConVarFlags::None, "Freezes the current camera position in place for occlusion culling.");
}
