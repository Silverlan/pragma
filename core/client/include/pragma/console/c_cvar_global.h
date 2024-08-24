/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_CVAR_GLOBAL_H__
#define __C_CVAR_GLOBAL_H__

#include <pragma/console/convars.h>
#include "pragma/console/c_cvar_global_functions.h"
#include <pragma/engine_info.hpp>

REGISTER_CONCOMMAND_CL(cl_entities, CMD_entities_cl, ConVarFlags::None, "Prints a list of all current clientside entities in the world.");
REGISTER_CONCOMMAND_CL(setpos, CMD_setpos, ConVarFlags::None, "Changes the position of the local player. Requires cheats to be enabled. Usage: setpos <x> <y> <z>");
REGISTER_CONCOMMAND_CL(getpos, CMD_getpos, ConVarFlags::None, "Prints the current position of the local player to the console.");
REGISTER_CONCOMMAND_CL(setcampos, CMD_setcampos, ConVarFlags::None, "Changes the current position of the camera.");
REGISTER_CONCOMMAND_CL(getcampos, CMD_getcampos, ConVarFlags::None, "Prints the current position of the camera.");
REGISTER_CONCOMMAND_CL(setang, CMD_setang, ConVarFlags::None, "Changes the angles of the local player. Requires cheats to be enabled. Usage: setang <pitch> <yaw> <roll>");
REGISTER_CONCOMMAND_CL(getang, CMD_getang, ConVarFlags::None, "Prints the current angles of the local player to the console.");
REGISTER_CONCOMMAND_CL(setcamang, CMD_setcamang, ConVarFlags::None, "Changes the current angles of the camera to the console.");
REGISTER_CONCOMMAND_CL(getcamang, CMD_getcamang, ConVarFlags::None, "Prints the current angles of the camera to the console.");
REGISTER_CONCOMMAND_CL(sound_play, CMD_sound_play, ConVarFlags::None, "Precaches and immediately plays the given sound file. Usage: sound_play <soundFile>");
REGISTER_CONCOMMAND_CL(sound_stop, CMD_sound_stop, ConVarFlags::None, "Stops all sounds that are currently playing.");
REGISTER_CONCOMMAND_CL(status, CMD_status_cl, ConVarFlags::None, "Prints information about the current connection to the console.");
REGISTER_CONCOMMAND_CL(screenshot, CMD_screenshot, ConVarFlags::None,
  "Writes the contents of the screen into a .tga-file in 'screenshots/'. The name of the file will start with the map's name, or '" + engine_info::get_identifier()
    + "' if no map is loaded. After the name follows an ID which is incremented each time. You can use 'screenshot 1' to take a screenshot without the GUI (Ingame only).");
REGISTER_CONCOMMAND_CL(thirdperson, CMD_thirdperson, ConVarFlags::None, "Toggles between first- and third-person mode.");
REGISTER_CONCOMMAND_CL(shader_reload, CMD_shader_reload, ConVarFlags::None, "Reloads a specific shader, or all registered shaders if no arguments are given. Usage: shader_reload <shaderName>");
REGISTER_CONCOMMAND_CL(shader_list, CMD_shader_list, ConVarFlags::None, "Prints a list of all currently loaded shaders");
REGISTER_CONCOMMAND_CL(shader_optimize, CMD_shader_optimize, ConVarFlags::None, "Uses LunarGLASS to optimize the specified shader.");
REGISTER_CONCOMMAND_CL(debug_light_shadowmap, CMD_debug_light_shadowmap, ConVarFlags::None, "Displays the depth map for the given light on screen. Call without arguments to turn the display off. Usage: debug_light_shadowmap <lightEntityIndex>");
REGISTER_CONCOMMAND_CL(debug_glow_bloom, CMD_debug_glow_bloom, ConVarFlags::None, "Displays the scene glow texture on screen. Usage: debug_glow_bloom <1/0>");
REGISTER_CONCOMMAND_CL(debug_hdr_bloom, CMD_debug_hdr_bloom, ConVarFlags::None, "Displays the scene bloom texture on screen. Usage: debug_hdr_bloom <1/0>");
REGISTER_CONCOMMAND_CL(debug_render_octree_dynamic_print, CMD_debug_render_octree_dynamic_print, ConVarFlags::None, "Prints the octree for dynamic objects to the console, or a file if a file name is specified.");
REGISTER_CONCOMMAND_CL(debug_render_octree_dynamic_find, CMD_debug_render_octree_dynamic_find, ConVarFlags::None, "Finds the specified entity in the octree for dynamic objects.");
REGISTER_CONCOMMAND_CL(debug_render_octree_static_print, CMD_debug_render_octree_static_print, ConVarFlags::None, "Prints the octree for static world geometry to the console, or a file if a file name is specified.");
REGISTER_CONCOMMAND_CL(debug_ai_schedule_print, CMD_debug_ai_schedule_print, ConVarFlags::None, "Prints the current schedule behavior tree for the specified NPC.");
REGISTER_CONCOMMAND_CL(debug_ai_schedule, CMD_debug_ai_schedule, ConVarFlags::None, "Prints the current schedule behavior tree for the specified NPC on screen.");
REGISTER_CONCOMMAND_CL(debug_aim_info, CMD_debug_aim_info, ConVarFlags::None, "Prints information about whatever the local player is looking at.");
REGISTER_CONCOMMAND_CL(debug_draw_line, CMD_debug_draw_line, ConVarFlags::None, "Draws a line from the current camera position to the specified target position");
REGISTER_CONCOMMAND_CL(debug_light_sources, Console::commands::debug_light_sources, ConVarFlags::None, "Prints debug information about all light sources in the scene.");
REGISTER_CONCOMMAND_CL(debug_gui_cursor, Console::commands::debug_gui_cursor, ConVarFlags::None, "Prints information about the GUI element currently hovered over by the cursor.");
REGISTER_CONCOMMAND_CL(debug_entity_render_buffer, Console::commands::debug_entity_render_buffer, ConVarFlags::None, "Prints debug information about an entity's render buffer.");
REGISTER_CONCOMMAND_CL(debug_font_glyph_map, Console::commands::debug_font_glyph_map, ConVarFlags::None, "Displays the glyph map for the specified font.");
REGISTER_CONCOMMAND_CL(debug_dump_font_glyph_map, Console::commands::debug_dump_font_glyph_map, ConVarFlags::None, "Dumps the glyph map for the specified font to an image file.");
REGISTER_CONCOMMAND_CL(debug_render_depth_buffer, Console::commands::debug_render_depth_buffer, ConVarFlags::None, "Draws the scene depth buffer to screen.");
REGISTER_CONCOMMAND_CL(debug_render_validation_error_enabled, Console::commands::debug_render_validation_error_enabled, ConVarFlags::None, "Enables or disables the specified validation error.");

REGISTER_CONCOMMAND_CL(debug_render_info, Console::commands::debug_render_info, ConVarFlags::None, "Prints some timing information to the console.");

REGISTER_CONCOMMAND_CL(debug_steam_audio_dump_scene, Console::commands::debug_steam_audio_dump_scene, ConVarFlags::None, "Saves the steam audio scene as OBJ file.");

REGISTER_CONCOMMAND_CL(flashlight, CMD_flashlight_toggle, ConVarFlags::None, "Toggles the flashlight.");
REGISTER_CONCOMMAND_CL(reloadmaterial, CMD_reloadmaterial, ConVarFlags::None, "Reloads the given material (and textures associated with it). Usage: reloadmaterial <materialFile>");
REGISTER_CONCOMMAND_CL(reloadmaterials, CMD_reloadmaterials, ConVarFlags::None, "Reloads all loaded materials and textures.");
REGISTER_CONCOMMAND_CL(debug_nav_path_start, CMD_debug_nav_path_start, ConVarFlags::None, "Sets a start point for a navigation path. Use debug_nav_path_end to set the end point.");
REGISTER_CONCOMMAND_CL(debug_nav_path_end, CMD_debug_nav_path_end, ConVarFlags::None, "Sets an end point for a navigation path. Use debug_nav_path_start to set the start point. The path will be drawn in the scene once both points have been set.");
REGISTER_CONCOMMAND_CL(debug_water, Console::commands::debug_water, ConVarFlags::None, "Displays the reflection, refraction and refraction depth map for the given water-entity on screen. Call without arguments to turn the display off. Usage: debug_light_water <waterEntityIndex>");
REGISTER_CONCOMMAND_CL(debug_ssao, Console::commands::debug_ssao, ConVarFlags::None, "Displays the ssao buffers to screen.");
REGISTER_CONCOMMAND_CL(debug_prepass, Console::commands::debug_prepass, ConVarFlags::None, "Displays prepass buffers to screen.");
REGISTER_CONCOMMAND_CL(debug_render_scene, Console::commands::debug_render_scene, ConVarFlags::None, "Displays scene to screen.");
REGISTER_CONCOMMAND_CL(cl_list, Console::commands::cl_list, ConVarFlags::None, "Prints a list of all clientside console commands to the console.");
REGISTER_CONCOMMAND_CL(cl_find, Console::commands::cl_find, ConVarFlags::None, "Finds similar console commands to whatever was given as argument.");
REGISTER_CONCOMMAND_CL(fps, CMD_fps, ConVarFlags::None, "Prints the current fps to the console.");
REGISTER_CONCOMMAND_CL(debug_audio_aux_effect, Console::commands::debug_audio_aux_effect, ConVarFlags::None, "Applies a global DSP effect. Usage: debug_audio_aux_effect <dspName> <gain>");
REGISTER_CONCOMMAND_CL(debug_audio_sounds, Console::commands::debug_audio_sounds, ConVarFlags::None, "Prints information about all active server- and clientside sounds to the console.");

REGISTER_CONCOMMAND_CL(vk_dump_limits, Console::commands::vk_dump_limits, ConVarFlags::None, "Dumps all vulkan limitations to 'vk_limits.txt'.");
REGISTER_CONCOMMAND_CL(vk_dump_features, Console::commands::vk_dump_features, ConVarFlags::None, "Dumps all vulkan features to 'vk_features.txt'.");
REGISTER_CONCOMMAND_CL(vk_dump_format_properties, Console::commands::vk_dump_format_properties, ConVarFlags::None, "Dumps all vulkan format properties to 'vk_format_properties.txt'.");
REGISTER_CONCOMMAND_CL(vk_dump_image_format_properties, Console::commands::vk_dump_image_format_properties, ConVarFlags::None, "Dumps all vulkan image format properties to 'vk_image_format_properties.txt'.");
REGISTER_CONCOMMAND_CL(vk_dump_layers, Console::commands::vk_dump_layers, ConVarFlags::None, "Dumps all available vulkan layers to 'vk_layers.txt'.");
REGISTER_CONCOMMAND_CL(vk_dump_extensions, Console::commands::vk_dump_extensions, ConVarFlags::None, "Dumps all available vulkan extensions to 'vk_extensions.txt'.");
REGISTER_CONCOMMAND_CL(vk_dump_memory_stats, Console::commands::vk_dump_memory_stats, ConVarFlags::None, "Dumps information about currently allocated GPU memory.");
REGISTER_CONCOMMAND_CL(vk_print_memory_stats, Console::commands::vk_print_memory_stats, ConVarFlags::None, "Prints information about currently allocated GPU memory to the console.");

REGISTER_CONCOMMAND_CL(debug_font, Console::commands::debug_font, ConVarFlags::None, "Displays the glyph map for the specified font. If no arguments are specified, all available fonts will be listed. Usage: debug_font <fontName>");
REGISTER_CONCOMMAND_CL(debug_texture_mipmaps, Console::commands::debug_texture_mipmaps, ConVarFlags::None, "Displays the mipmaps of the given texture. Usage: debug_texture_mipmaps <texturePath>");
REGISTER_CONCOMMAND_CL(debug_hitboxes, Console::commands::debug_hitboxes, ConVarFlags::None, "Displays the hitboxes for the entity under the crosshair, or the entity with the given name (If it was specified.).");
REGISTER_CONCOMMAND_CL(debug_dump_component_properties, Console::commands::debug_dump_component_properties, ConVarFlags::None, "Dumps entity component property values to the console.");

REGISTER_CONCOMMAND_CL(debug_pbr_ibl, Console::commands::debug_pbr_ibl, ConVarFlags::None, "Displays the irradiance, prefilter and brdf map for the closest cubemap.");
REGISTER_CONCOMMAND_CL(debug_particle_alpha_mode, Console::commands::debug_particle_alpha_mode, ConVarFlags::None, "Specifies the blend mode arguments for particle systems that use the \
'custom' alpha mode. Argument order: <srcColorBlendFactor> <dstColorBlendFactor> <srcAlphaBlendFactor> <dstAlphaBlendFactor> <opColor> <opAlpha>.\n\
Blend factor options: zero, one, src_color, one_minus_src_color, dst_color, one_minus_dst_color, src_alpha, one_minus_src_alpha, dst_alpha, one_minus_dst_alpha, constant_color, \
one_minus_constant_color, constant_alpha, one_minus_constant_alpha, src_alpha_saturate, src1_color, one_minus_src1_color, src1_alpha, one_minus_src1_alpha\n\
Operation options: add, subtract, reverse_subtract, min, max");

#ifdef _DEBUG
REGISTER_CONCOMMAND_CL(cl_dump_sounds, CMD_cl_dump_sounds, ConVarFlags::None, "Prints information about all active sounds to the console.");
REGISTER_CONCOMMAND_CL(cl_dump_netmessages, CMD_cl_dump_netmessages, ConVarFlags::None, "Prints all registered netmessages to the console.");
#endif

REGISTER_CONCOMMAND_CL(cl_gpu_timer_queries_dump, Console::commands::cl_gpu_timer_queries_dump, ConVarFlags::None, "Prints all timer query results to the console.");

REGISTER_CONCOMMAND_CL(cl_steam_audio_reload_scene, Console::commands::cl_steam_audio_reload_scene, ConVarFlags::None, "Reloads the steam audio scene cache.");

REGISTER_CONCOMMAND_CL(map_build_reflection_probes, Console::commands::map_build_reflection_probes, ConVarFlags::None,
  "Build all reflection probes in the map. Use the '-rebuild' argument to clear all current IBL textures first. Use 'debug_pbr_ibl' to check the probes after they have been built.");
REGISTER_CONCOMMAND_CL(map_rebuild_lightmaps, Console::commands::map_rebuild_lightmaps, ConVarFlags::None, "Rebuilds the lightmaps for the current map. Note that this will only work if the map was compiled with lightmap uvs.");

REGISTER_CONCOMMAND_CL(util_convert_cubemap_to_equirectangular_image, Console::commands::util_convert_cubemap_to_equirectangular_image, ConVarFlags::None, "Converts a cubemap to a equirectangular image.");
REGISTER_CONCOMMAND_CL(util_convert_equirectangular_image_to_cubemap, Console::commands::util_convert_equirectangular_image_to_cubemap, ConVarFlags::None, "Converts a equirectangular image to a cubemap.");

#endif
