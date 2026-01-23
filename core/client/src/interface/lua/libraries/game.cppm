// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.game;

export import pragma.lua;

export namespace Lua {
	namespace game {
		namespace Client {
			DLLCLIENT int open_dropped_file(lua::State *l);
			DLLCLIENT int set_gravity(lua::State *l);
			DLLCLIENT int get_gravity(lua::State *l);
			DLLCLIENT int load_model(lua::State *l);
			DLLCLIENT int create_model(lua::State *l);
			DLLCLIENT int get_action_input(lua::State *l);
			DLLCLIENT int set_action_input(lua::State *l);
			DLLCLIENT int queue_scene_for_rendering(lua::State *l);
			DLLCLIENT int render_scenes(lua::State *l);
			DLLCLIENT int set_debug_render_filter(lua::State *l);
			DLLCLIENT int update_render_buffers(lua::State *l);
			DLLCLIENT int create_scene(lua::State *l);
			DLLCLIENT int get_render_scene(lua::State *l);
			DLLCLIENT int get_render_scene_camera(lua::State *l);
			DLLCLIENT int get_scene(lua::State *l);
			DLLCLIENT int get_scene_by_index(lua::State *l);
			DLLCLIENT int get_scene_camera(lua::State *l);
			DLLCLIENT int get_draw_command_buffer(lua::State *l);
			DLLCLIENT int get_setup_command_buffer(lua::State *l);
			DLLCLIENT int flush_setup_command_buffer(lua::State *l);
			DLLCLIENT int get_camera_position(lua::State *l);
			DLLCLIENT int get_render_clip_plane(lua::State *l);
			DLLCLIENT int set_render_clip_plane(lua::State *l);

			DLLCLIENT int get_debug_buffer(lua::State *l);
			DLLCLIENT int get_time_buffer(lua::State *l);
			DLLCLIENT int get_csm_buffer(lua::State *l);
			DLLCLIENT int get_render_settings_descriptor_set(lua::State *l);

			DLLCLIENT int load_map(lua::State *l);
			DLLCLIENT int build_reflection_probes(lua::State *l);

			DLLCLIENT int test(lua::State *l);
			DLLCLIENT int set_render_stats_enabled(lua::State *l);
			DLLCLIENT int get_queued_render_scenes(lua::State *l);
		};
	};
};
