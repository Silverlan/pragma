// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui;
export import :gui.checkbox;
export import :gui.choice_list;
export import :gui.chromium_page;
export import :gui.command_line_entry;
export import :gui.console;
export import :gui.container;
export import :gui.debug_depth_texture;
export import :gui.debug_hdr_bloom;
export import :gui.debug_mipmaps;
export import :gui.debug_msaa_texture;
export import :gui.debug_shadow_map;
export import :gui.debug_ssao;
export import :gui.detachable;
export import :gui.fps;
export import :gui.frame;
export import :gui.grid_panel;
export import :gui.icon;
export import :gui.image_slide_show;
export import :gui.key_entry;
export import :gui.line_graph;
export import :gui.load_screen;
export import :gui.lua_base;
export import :gui.lua_handle_wrapper;
export import :gui.lua_interface;
export import :gui.lua_skin;
export import :gui.main_menu;
export import :gui.main_menu_base;
export import :gui.main_menu_credits;
export import :gui.main_menu_load_game;
export import :gui.main_menu_mods;
export import :gui.main_menu_new_game;
export import :gui.main_menu_options;
export import :gui.message_box;
export import :gui.net_graph;
export import :gui.options_list;
export import :gui.progress_bar;
export import :gui.scroll_container;
export import :gui.server_browser;
export import :gui.silk_icon;
export import :gui.slider;
export import :gui.snap_area;
export import :gui.table;
export import :gui.textured_cubemap;
export import :gui.transformable;
export import :gui.tree_list;

export {
	bool load_skin(const std::string &skinName);
};
