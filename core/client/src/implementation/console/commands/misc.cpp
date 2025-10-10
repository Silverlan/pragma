// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.register_commands;

DLLCLIENT void CMD_lua_reload_entity(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_lua_reload_weapon(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_lua_reload_entities(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_lua_reload_weapons(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

namespace {
	using namespace pragma::console::client;
	auto UVN = register_command("cl_entities", &CMD_entities_cl, ConVarFlags::None, "Prints a list of all current clientside entities in the world.");
	auto UVN = register_command("setpos", &CMD_setpos, ConVarFlags::None, "Changes the position of the local player. Requires cheats to be enabled. Usage: setpos <x> <y> <z>");
	auto UVN = register_command("getpos", &CMD_getpos, ConVarFlags::None, "Prints the current position of the local player to the console.");
	auto UVN = register_command("setcampos", &CMD_setcampos, ConVarFlags::None, "Changes the current position of the camera.");
	auto UVN = register_command("getcampos", &CMD_getcampos, ConVarFlags::None, "Prints the current position of the camera.");
	auto UVN = register_command("setang", &CMD_setang, ConVarFlags::None, "Changes the angles of the local player. Requires cheats to be enabled. Usage: setang <pitch> <yaw> <roll>");
	auto UVN = register_command("getang", &CMD_getang, ConVarFlags::None, "Prints the current angles of the local player to the console.");
	auto UVN = register_command("setcamang", &CMD_setcamang, ConVarFlags::None, "Changes the current angles of the camera to the console.");
	auto UVN = register_command("getcamang", &CMD_getcamang, ConVarFlags::None, "Prints the current angles of the camera to the console.");
	auto UVN = register_command("sound_play", &CMD_sound_play, ConVarFlags::None, "Precaches and immediately plays the given sound file. Usage: sound_play <soundFile>");
	auto UVN = register_command("sound_stop", &CMD_sound_stop, ConVarFlags::None, "Stops all sounds that are currently playing.");
	auto UVN = register_command("status", &CMD_status_cl, ConVarFlags::None, "Prints information about the current connection to the console.");
	auto UVN = register_command("screenshot", &CMD_screenshot, ConVarFlags::None,
	"Writes the contents of the screen into a .tga-file in 'screenshots/'. The name of the file will start with the map's name, or '" + engine_info::get_identifier()
		+ "' if no map is loaded. After the name follows an ID which is incremented each time. You can use 'screenshot 1' to take a screenshot without the GUI (Ingame only).");
	auto UVN = register_command("thirdperson", &CMD_thirdperson, ConVarFlags::None, "Toggles between first- and third-person mode.");
	auto UVN = register_command("shader_reload", &CMD_shader_reload, ConVarFlags::None, "Reloads a specific shader, or all registered shaders if no arguments are given. Usage: shader_reload <shaderName>");
	auto UVN = register_command("shader_list", &CMD_shader_list, ConVarFlags::None, "Prints a list of all currently loaded shaders");
	auto UVN = register_command("shader_optimize", &CMD_shader_optimize, ConVarFlags::None, "Uses LunarGLASS to optimize the specified shader.");
	auto UVN = register_command("debug_light_shadowmap", &CMD_debug_light_shadowmap, ConVarFlags::None, "Displays the depth map for the given light on screen. Call without arguments to turn the display off. Usage: debug_light_shadowmap <lightEntityIndex>");
	auto UVN = register_command("debug_hdr_bloom", &CMD_debug_hdr_bloom, ConVarFlags::None, "Displays the scene bloom texture on screen. Usage: debug_hdr_bloom <1/0>");
	auto UVN = register_command("debug_render_octree_dynamic_print", &CMD_debug_render_octree_dynamic_print, ConVarFlags::None, "Prints the octree for dynamic objects to the console, or a file if a file name is specified.");
	auto UVN = register_command("debug_render_octree_dynamic_find", &CMD_debug_render_octree_dynamic_find, ConVarFlags::None, "Finds the specified entity in the octree for dynamic objects.");
	auto UVN = register_command("debug_render_octree_static_print", &CMD_debug_render_octree_static_print, ConVarFlags::None, "Prints the octree for static world geometry to the console, or a file if a file name is specified.");
	auto UVN = register_command("debug_ai_schedule_print", &CMD_debug_ai_schedule_print, ConVarFlags::None, "Prints the current schedule behavior tree for the specified NPC.");
	auto UVN = register_command("debug_ai_schedule", &CMD_debug_ai_schedule, ConVarFlags::None, "Prints the current schedule behavior tree for the specified NPC on screen.");
	auto UVN = register_command("debug_aim_info", &CMD_debug_aim_info, ConVarFlags::None, "Prints information about whatever the local player is looking at.");
	auto UVN = register_command("debug_draw_line", &CMD_debug_draw_line, ConVarFlags::None, "Draws a line from the current camera position to the specified target position");
	auto UVN = register_command("debug_render_depth_buffer", &Console::commands::debug_render_depth_buffer, ConVarFlags::None, "Draws the scene depth buffer to screen.");
	auto UVN = register_command("debug_render_validation_error_enabled", &Console::commands::debug_render_validation_error_enabled, ConVarFlags::None, "Enables or disables the specified validation error.");

	auto UVN = register_command("flashlight", &CMD_flashlight_toggle, ConVarFlags::None, "Toggles the flashlight.");
	auto UVN = register_command("reloadmaterial", &CMD_reloadmaterial, ConVarFlags::None, "Reloads the given material (and textures associated with it). Usage: reloadmaterial <materialFile>");
	auto UVN = register_command("reloadmaterials", &CMD_reloadmaterials, ConVarFlags::None, "Reloads all loaded materials and textures.");
	auto UVN = register_command("debug_nav_path_start", &CMD_debug_nav_path_start, ConVarFlags::None, "Sets a start point for a navigation path. Use debug_nav_path_end to set the end point.");
	auto UVN = register_command("debug_nav_path_end", &CMD_debug_nav_path_end, ConVarFlags::None, "Sets an end point for a navigation path. Use debug_nav_path_start to set the start point. The path will be drawn in the scene once both points have been set.");
	auto UVN = register_command("cl_list", &Console::commands::cl_list, ConVarFlags::None, "Prints a list of all clientside console commands to the console.");
	auto UVN = register_command("cl_find", &Console::commands::cl_find, ConVarFlags::None, "Finds similar console commands to whatever was given as argument.");
	auto UVN = register_command("fps", &CMD_fps, ConVarFlags::None, "Prints the current fps to the console.");

	auto UVN = register_command("vk_dump_limits", &Console::commands::vk_dump_limits, ConVarFlags::None, "Dumps all vulkan limitations to 'vk_limits.txt'.");
	auto UVN = register_command("vk_dump_features", &Console::commands::vk_dump_features, ConVarFlags::None, "Dumps all vulkan features to 'vk_features.txt'.");
	auto UVN = register_command("vk_dump_format_properties", &Console::commands::vk_dump_format_properties, ConVarFlags::None, "Dumps all vulkan format properties to 'vk_format_properties.txt'.");
	auto UVN = register_command("vk_dump_image_format_properties", &Console::commands::vk_dump_image_format_properties, ConVarFlags::None, "Dumps all vulkan image format properties to 'vk_image_format_properties.txt'.");
	auto UVN = register_command("vk_dump_layers", &Console::commands::vk_dump_layers, ConVarFlags::None, "Dumps all available vulkan layers to 'vk_layers.txt'.");
	auto UVN = register_command("vk_dump_extensions", &Console::commands::vk_dump_extensions, ConVarFlags::None, "Dumps all available vulkan extensions to 'vk_extensions.txt'.");
	auto UVN = register_command("vk_dump_memory_stats", &Console::commands::vk_dump_memory_stats, ConVarFlags::None, "Dumps information about currently allocated GPU memory.");
	auto UVN = register_command("vk_print_memory_stats", &Console::commands::vk_print_memory_stats, ConVarFlags::None, "Prints information about currently allocated GPU memory to the console.");

	auto UVN = register_command("debug_dump_component_properties", &Console::commands::debug_dump_component_properties, ConVarFlags::None, "Dumps entity component property values to the console.");

	auto UVN = register_command("debug_pbr_ibl", &Console::commands::debug_pbr_ibl, ConVarFlags::None, "Displays the irradiance, prefilter and brdf map for the closest cubemap.");

	#ifdef _DEBUG
	auto UVN = register_command("cl_dump_sounds", &CMD_cl_dump_sounds, ConVarFlags::None, "Prints information about all active sounds to the console.");
	auto UVN = register_command("cl_dump_netmessages", &CMD_cl_dump_netmessages, ConVarFlags::None, "Prints all registered netmessages to the console.");
	#endif
	
	auto UVN = register_command("lua_reload_entity", &CMD_lua_reload_entity, ConVarFlags::None, "Reloads the scripts for the given entity class. Usage: lua_reload_entity <className>");
	auto UVN = register_command("lua_reload_weapon", &CMD_lua_reload_weapon, ConVarFlags::None, "Reloads the scripts for the given weapon class. Usage: lua_reload_weapon <className>");
	auto UVN = register_command("lua_reload_entities", &CMD_lua_reload_entities, ConVarFlags::None, "Reloads the scripts for all registered lua entity classes.");
	auto UVN = register_command("lua_reload_weapons", &CMD_lua_reload_weapons, ConVarFlags::None, "Reloads the scripts for all registered lua weapon classes.");
}
