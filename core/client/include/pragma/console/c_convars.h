// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

////////////////////////////////////////////////////////////////////
////// Contains declarations for clientside convars / concmds //////
////// This is included in the SHARED library, NOT the client //////
///// ConCmd definitions have to be inside the client library //////
////////////////////////////////////////////////////////////////////
#ifdef ENGINE_NETWORK
#ifndef __C_CONVARS_H__
#define __C_CONVARS_H__
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>
#include "pragma/console/cengine_convars.h"
#include "pragma/console/c_cvar_server.h"
#include "pragma/console/c_cvar_global.h"
#include "pragma/console/c_cvar_movement.h"
#include "pragma/console/c_cvar_settings.h"
REGISTER_CONVAR_CL(cl_allowdownload, udm::Type::Boolean, "1", ConVarFlags::Archive, "");
REGISTER_CONVAR_CL(cl_allowupload, udm::Type::Boolean, "1", ConVarFlags::Archive, "");
REGISTER_CONVAR_CL(cl_fov, udm::Type::Float, "75", ConVarFlags::Archive, "Horizontal world field of view.");
REGISTER_CONVAR_CL(cl_fov_viewmodel, udm::Type::Float, "70", ConVarFlags::Archive, "Horizontal view model field of view.");
REGISTER_CONVAR_CL(debug_nav_show_meshes, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Shows the navigation mesh for this map.");
REGISTER_CONVAR_CL(debug_physics_draw, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Draws various physics information as wireframe.");
REGISTER_CONVAR_CL(sv_debug_physics_draw, udm::Type::Boolean, "0", ConVarFlags::Cheat | ConVarFlags::Singleplayer, "Draws various serverside physics information as wireframe.");
REGISTER_CONVAR_CL(debug_bsp_nodes, udm::Type::Boolean, "0", ConVarFlags::Cheat,
  "Prints and draws various debug information about the binary space partitioning of the level (if available). 1 = Draw leaf of camera location and all visible leaves, 2 = Same, but doesn't update continuously.");
REGISTER_CONVAR_CL(debug_bsp_lock, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Locks BSP culling so it doesn't change when the camera moves.");
REGISTER_CONVAR_CL(debug_render_octree_static_draw, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Draws the octree for static world geometry.");
REGISTER_CONVAR_CL(debug_render_octree_dynamic_draw, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Draws the octree for dynamic objects.");
REGISTER_CONVAR_CL(debug_ai_navigation, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Displays the current navigation path for all NPCs.");
REGISTER_CONVAR_CL(debug_steam_audio_probe_boxes, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Displays the sound probe spheres for the current map.");
REGISTER_CONVAR_CL(cl_fps_decay_factor, udm::Type::Float, "0.8", ConVarFlags::None, "How slowly to decay the previous fps.");

REGISTER_CONVAR_CL(debug_particle_blob_show_neighbor_links, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Displays the links to adjacent neighbors for blob particles.");

REGISTER_CONVAR_CL(debug_render_lock_render_queues, udm::Type::Boolean, "0", ConVarFlags::None, "Locks the current render queues and effectively disables the render queue thread.");
REGISTER_CONVAR_CL(debug_render_lock_render_command_buffers, udm::Type::Boolean, "0", ConVarFlags::None, "Locks the current render command buffers and causes them to not be re-created every frame.");
REGISTER_CONVAR_CL(debug_render_normals, udm::Type::Boolean, "0", ConVarFlags::Cheat, "Displays all normals as lines in the world. 0 = Disabled, 1 = Display normals, 2 = Display normals, tangents and bitangents, 3 = Display face normals, 4 = Display tangents, 5 = Display bitangents.");
REGISTER_CONVAR_CL(cl_show_fps, udm::Type::Boolean, "0", ConVarFlags::None, "Displays the current fps at the top left of the screen if enabled.");

REGISTER_CONVAR_CL(render_vsync_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "1 = Vertical sync enabled, 0 = disabled.");
REGISTER_CONVAR_CL(render_normalmapping_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "1 = Normal mapping enabled, 0 = disabled.");
REGISTER_CONVAR_CL(render_csm_max_distance, udm::Type::Float, "8192", ConVarFlags::Archive, "The maximum distance at which csm shadows are casted. A larger distance means lower quality.");
REGISTER_CONVAR_CL(render_texture_filtering, udm::Type::UInt8, "1", ConVarFlags::Archive, "0 = Nearest-Neighbor, 1 = Bilinear, 2 = trilinear.");
REGISTER_CONVAR_CL(render_hdr_max_exposure, udm::Type::Float, "8", ConVarFlags::Archive, "Expore will be clamped at this value. Can be overwritten by the map.");
REGISTER_CONVAR_CL(render_draw_scene, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Scene isn't drawn.");
REGISTER_CONVAR_CL(render_draw_world, udm::Type::UInt8, "1", ConVarFlags::Cheat, "1 = Default, 2 = Wireframe, 0 = Don't draw world.");
REGISTER_CONVAR_CL(render_draw_static, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Don't draw level geometry.");
REGISTER_CONVAR_CL(render_draw_dynamic, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Don't draw props.");
REGISTER_CONVAR_CL(render_draw_translucent, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Don't draw translucent objects.");
REGISTER_CONVAR_CL(render_draw_sky, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Sky isn't drawn.");
REGISTER_CONVAR_CL(render_draw_water, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Water isn't drawn.");
REGISTER_CONVAR_CL(render_draw_view, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = View-Models aren't drawn.");
REGISTER_CONVAR_CL(render_draw_particles, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Particles aren't drawn.");
REGISTER_CONVAR_CL(render_draw_glow, udm::Type::Boolean, "1", ConVarFlags::Cheat, "1 = Default, 0 = Glow-effects aren't drawn.");
REGISTER_CONVAR_CL(render_clear_scene, udm::Type::Boolean, "0", static_cast<ConVarFlags>(0), "0 = Default, 1 = Screen is cleared before scene is rendered.");
REGISTER_CONVAR_CL(render_clear_scene_color, udm::Type::Vector4i, "0 0 0 255", static_cast<ConVarFlags>(0), "Clear color to use if render_clear_scene is set to 1.");
REGISTER_CONVAR_CL(render_forwardplus_tile_size, udm::Type::UInt32, "16", static_cast<ConVarFlags>(0), "Changes the tile size used for the light culling in forward+.");
REGISTER_CONVAR_CL(render_technique, udm::Type::UInt8, "0", static_cast<ConVarFlags>(0), "0 = Rasterization, 1 = Raytracing");
REGISTER_CONVAR_CL(sky_override, udm::Type::String, "", static_cast<ConVarFlags>(0), "Replaces the material for all current skyboxes with the specified material.");

REGISTER_CONVAR_CL(render_pp_nightvision, udm::Type::Boolean, "0", ConVarFlags::None, "Enables or disables night vision.");
REGISTER_CONVAR_CL(render_pp_nightvision_luminance_threshold, udm::Type::Float, "0.2", ConVarFlags::Archive, "Changes the luminance threshold for the night vision post-processing effect.");
REGISTER_CONVAR_CL(render_pp_nightvision_color_amplification, udm::Type::Float, "4", ConVarFlags::Archive, "Amplification factor for pixels with low luminance.");
REGISTER_CONVAR_CL(render_pp_nightvision_noise_texture, udm::Type::String, "effects/noise1", ConVarFlags::Archive, "Noise texture to use for the night vision post-processing effect.");

REGISTER_CONVAR_CL(render_bloom_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enables or disables bloom and glow effects.");
REGISTER_CONVAR_CL(render_bloom_amount, udm::Type::UInt32, "9", ConVarFlags::Archive, "Number of render passes to use for bloom.");
REGISTER_CONVAR_CL(render_bloom_resolution, udm::Type::UInt32, "512", ConVarFlags::Archive, "The width for the bloom texture. The height will be calculated using the aspect ratio of the renderer.");

REGISTER_CONVAR_CL(debug_occlusion_culling_freeze_camera, udm::Type::Boolean, "0", ConVarFlags::None, "Freezes the current camera position in place for occlusion culling.");

#endif
#endif
