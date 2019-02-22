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
REGISTER_CONVAR_CL(cl_allowdownload,"1",ConVarFlags::Archive,"");
REGISTER_CONVAR_CL(cl_allowupload,"1",ConVarFlags::Archive,"");
REGISTER_CONVAR_CL(cl_fov,"75",ConVarFlags::Archive,"Horizontal world field of view.");
REGISTER_CONVAR_CL(cl_fov_viewmodel,"70",ConVarFlags::Archive,"Horizontal view model field of view.");
REGISTER_CONVAR_CL(debug_csm_show_cascades,"0",ConVarFlags::Cheat,"Displays the cascades in different colors.");
REGISTER_CONVAR_CL(debug_light_depth,"0",ConVarFlags::Cheat,"1 = Display depth values from shadow maps in light sources, 2 = Display depth values in shadow space.");
REGISTER_CONVAR_CL(debug_nav_show_meshes,"0",ConVarFlags::Cheat,"Shows the navigation mesh for this map.");
REGISTER_CONVAR_CL(debug_physics_draw,"0",ConVarFlags::Cheat,"Draws various physics information as wireframe. 1 = Draw all available information, 2 = Draw physics geometry only, 3 = Draw constraints only, 4 = Draw normals only.");
REGISTER_CONVAR_CL(debug_bsp_nodes,"0",ConVarFlags::Cheat,"Prints and draws various debug information about the binary space partitioning of the level (if available). 1 = Draw leaf of camera location and all visible leaves, 2 = Same, but doesn't update continuously.");
REGISTER_CONVAR_CL(debug_bsp_lock,"0",ConVarFlags::Cheat,"Locks BSP culling so it doesn't change when the camera moves.");
REGISTER_CONVAR_CL(debug_render_octree_static_draw,"0",ConVarFlags::Cheat,"Draws the octree for static world geometry.");
REGISTER_CONVAR_CL(debug_render_octree_dynamic_draw,"0",ConVarFlags::Cheat,"Draws the octree for dynamic objects.");
REGISTER_CONVAR_CL(debug_render_depth_buffer,"0",ConVarFlags::Cheat,"Draws the scene depth buffer to screen.");
REGISTER_CONVAR_CL(debug_forwardplus_heatmap,"0",ConVarFlags::Cheat,"Renders a heatmap of the visible lights for forward+ rendering.");
REGISTER_CONVAR_CL(debug_ai_navigation,"0",ConVarFlags::Cheat,"Displays the current navigation path for all NPCs.");
REGISTER_CONVAR_CL(debug_steam_audio_probe_boxes,"0",ConVarFlags::Cheat,"Displays the sound probe spheres for the current map.");

REGISTER_CONVAR_CL(debug_particle_blob_show_neighbor_links,"0",ConVarFlags::Cheat,"Displays the links to adjacent neighbors for blob particles.");

REGISTER_CONVAR_CL(debug_render_normals,"0",ConVarFlags::Cheat,"Displays all normals as lines in the world. 0 = Disabled, 1 = Display normals, 2 = Display normals, tangents and bitangents, 3 = Display face normals, 4 = Display tangents, 5 = Display bitangents.");
REGISTER_CONVAR_CL(cl_show_fps,"0",ConVarFlags::None,"Displays the current fps at the top left of the screen if enabled.");

REGISTER_CONVAR_CL(render_unlit,"0",ConVarFlags::Cheat,"1 = World will be rendered fullbright, 0 = World will be rendered normally (With light effects).");
REGISTER_CONVAR_CL(render_vsync_enabled,"1",ConVarFlags::Archive,"1 = Vertical sync enabled, 0 = disabled.");
REGISTER_CONVAR_CL(render_normalmapping_enabled,"1",ConVarFlags::Archive,"1 = Normal mapping enabled, 0 = disabled.");
REGISTER_CONVAR_CL(render_csm_max_distance,"8192",ConVarFlags::Archive,"The maximum distance at which csm shadows are casted. A larger distance means lower quality.");
REGISTER_CONVAR_CL(render_texture_filtering,"1",ConVarFlags::Archive,"0 = Nearest-Neighbor, 1 = Bilinear, 2 = trilinear.");
REGISTER_CONVAR_CL(render_hdr_max_exposure,"8",ConVarFlags::Archive,"Expore will be clamped at this value. Can be overwritten by the map.");
REGISTER_CONVAR_CL(render_draw_scene,"1",ConVarFlags::Cheat,"1 = Default, 0 = Scene isn't drawn.");
REGISTER_CONVAR_CL(render_draw_world,"1",ConVarFlags::Cheat,"1 = Default, 2 = Wireframe, 0 = Don't draw world.");
REGISTER_CONVAR_CL(render_draw_sky,"1",ConVarFlags::Cheat,"1 = Default, 0 = Sky isn't drawn.");
REGISTER_CONVAR_CL(render_draw_water,"1",ConVarFlags::Cheat,"1 = Default, 0 = Water isn't drawn.");
REGISTER_CONVAR_CL(render_draw_view,"1",ConVarFlags::Cheat,"1 = Default, 0 = View-Models aren't drawn.");
REGISTER_CONVAR_CL(render_draw_particles,"1",ConVarFlags::Cheat,"1 = Default, 0 = Particles aren't drawn.");
REGISTER_CONVAR_CL(render_draw_glow,"1",ConVarFlags::Cheat,"1 = Default, 0 = Glow-effects aren't drawn.");
REGISTER_CONVAR_CL(render_draw_translucent,"1",ConVarFlags::Cheat,"1 = Default, 0 = Translucent geometry isn't drawn.");
REGISTER_CONVAR_CL(render_clear_scene,"0",static_cast<ConVarFlags>(0),"0 = Default, 1 = Screen is cleared before scene is rendered.");
REGISTER_CONVAR_CL(render_clear_scene_color,"0 0 0 255",static_cast<ConVarFlags>(0),"Clear color to use if render_clear_scene is set to 1.");

REGISTER_CONVAR_CL(render_pp_nightvision,"0",ConVarFlags::None,"Enables or disables night vision.");
REGISTER_CONVAR_CL(render_pp_nightvision_luminance_threshold,"0.2",ConVarFlags::Archive,"Changes the luminance threshold for the night vision post-processing effect.");
REGISTER_CONVAR_CL(render_pp_nightvision_color_amplification,"4",ConVarFlags::Archive,"Amplification factor for pixels with low luminance.");
REGISTER_CONVAR_CL(render_pp_nightvision_noise_texture,"effects/noise1",ConVarFlags::Archive,"Noise texture to use for the night vision post-processing effect.");

#endif
#endif