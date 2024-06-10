/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_CVAR_GLOBAL_FUNCTIONS_H__
#define __C_CVAR_GLOBAL_FUNCTIONS_H__

#include "pragma/clientdefinitions.h"
#include "pragma/networkdefinitions.h"

DLLCLIENT void CMD_entities_cl(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
DLLCLIENT void CMD_setpos(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
DLLCLIENT void CMD_getpos(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_getcampos(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_setang(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_getang(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_getcamang(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_sound_play(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_sound_stop(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_status_cl(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_screenshot(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_shader_reload(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_shader_list(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_shader_optimize(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_light_shadowmap(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_glow_bloom(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_hdr_bloom(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_render_octree_dynamic_print(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_render_octree_dynamic_find(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_render_octree_static_print(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_ai_schedule_print(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_ai_schedule(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_aim_info(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_draw_line(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

DLLCLIENT void CMD_thirdperson(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_flashlight_toggle(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_reloadmaterial(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_reloadmaterials(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_nav_path_start(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_debug_nav_path_end(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_fps(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#ifdef _DEBUG
DLLCLIENT void CMD_cl_dump_sounds(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_cl_dump_netmessages(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#endif

namespace Console {
	namespace commands {
		DLLCLIENT void cl_list(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void cl_find(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_render_info(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		DLLCLIENT void vk_dump_limits(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void vk_dump_features(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void vk_dump_format_properties(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void vk_dump_image_format_properties(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void vk_dump_layers(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void vk_dump_extensions(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void vk_dump_memory_stats(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		DLLCLIENT void vk_print_memory_stats(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		DLLCLIENT void debug_texture_mipmaps(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_pbr_ibl(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_font(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_hitboxes(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_water(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_ssao(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_prepass(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_render_scene(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_light_sources(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_gui_cursor(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_entity_render_buffer(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_steam_audio_dump_scene(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_lightmaps(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_particle_alpha_mode(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_font_glyph_map(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_dump_font_glyph_map(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_render_depth_buffer(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_render_validation_error_enabled(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_dump_component_properties(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		DLLCLIENT void debug_audio_aux_effect(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void debug_audio_sounds(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void cl_steam_audio_reload_scene(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		DLLCLIENT void cl_gpu_timer_queries_dump(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		DLLCLIENT void map_build_reflection_probes(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void map_rebuild_lightmaps(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		DLLCLIENT void util_convert_cubemap_to_equirectangular_image(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		DLLCLIENT void util_convert_equirectangular_image_to_cubemap(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

	};
};

#endif
