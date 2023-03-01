/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/wisilkicon.h"
#include <sharedutils/util_string.h>

extern ClientState *client;
LINK_WGUI_TO_CLASS(WISilkIcon, WISilkIcon);

WISilkIcon::WISilkIcon() : WIIcon() {}

WISilkIcon::~WISilkIcon() {}

void WISilkIcon::Initialize()
{
	WIIcon::Initialize();
	SetMaterial("wgui/silkicons");
	SetSize(16, 16);

	SetIcon("accept");
}

struct DLLCLIENT SilkIcon {
	SilkIcon(const char *pName, unsigned int pRow, unsigned int pColumn) : name(pName), row(pRow), col(pColumn) {}
	const char *name;
	unsigned int row;
	unsigned int col;
};

static SilkIcon icons[] = {SilkIcon("accept", 0, 0), SilkIcon("add", 0, 1), SilkIcon("anchor", 0, 2), SilkIcon("application", 1, 0), SilkIcon("application_add", 1, 1), SilkIcon("application_cascade", 2, 0), SilkIcon("application_delete", 2, 1), SilkIcon("application_double", 1, 2),
  SilkIcon("application_edit", 2, 2), SilkIcon("application_error", 0, 3), SilkIcon("application_form", 1, 3), SilkIcon("application_form_add", 0, 4), SilkIcon("application_form_delete", 2, 3), SilkIcon("application_form_edit", 1, 4), SilkIcon("application_form_magnify", 0, 5),
  SilkIcon("application_get", 0, 6), SilkIcon("application_go", 1, 5), SilkIcon("application_home", 2, 4), SilkIcon("application_key", 2, 5), SilkIcon("application_lightning", 1, 6), SilkIcon("application_link", 2, 6), SilkIcon("application_osx", 3, 0),
  SilkIcon("application_osx_terminal", 4, 0), SilkIcon("application_put", 3, 1), SilkIcon("application_side_boxes", 4, 1), SilkIcon("application_side_contract", 5, 0), SilkIcon("application_side_expand", 3, 2), SilkIcon("application_side_list", 5, 1),
  SilkIcon("application_side_tree", 6, 0), SilkIcon("application_split", 3, 3), SilkIcon("application_tile_horizontal", 4, 2), SilkIcon("application_tile_vertical", 5, 2), SilkIcon("application_view_columns", 3, 4), SilkIcon("application_view_detail", 6, 1),
  SilkIcon("application_view_gallery", 4, 3), SilkIcon("application_view_icons", 6, 2), SilkIcon("application_view_list", 4, 4), SilkIcon("application_view_tile", 5, 3), SilkIcon("application_xp", 3, 5), SilkIcon("application_xp_terminal", 5, 4), SilkIcon("arrow_branch", 6, 3),
  SilkIcon("arrow_divide", 3, 6), SilkIcon("arrow_down", 4, 5), SilkIcon("arrow_in", 4, 6), SilkIcon("arrow_inout", 5, 5), SilkIcon("arrow_join", 6, 4), SilkIcon("arrow_left", 6, 5), SilkIcon("arrow_merge", 5, 6), SilkIcon("arrow_out", 6, 6), SilkIcon("arrow_redo", 0, 7),
  SilkIcon("arrow_refresh", 1, 7), SilkIcon("arrow_refresh_small", 0, 8), SilkIcon("arrow_right", 1, 8), SilkIcon("arrow_rotate_anticlockwise", 2, 7), SilkIcon("arrow_rotate_clockwise", 0, 9), SilkIcon("arrow_switch", 1, 9), SilkIcon("arrow_turn_left", 2, 8),
  SilkIcon("arrow_turn_right", 3, 7), SilkIcon("arrow_undo", 0, 10), SilkIcon("arrow_up", 0, 11), SilkIcon("asterisk_orange", 1, 10), SilkIcon("asterisk_yellow", 2, 9), SilkIcon("attach", 3, 8), SilkIcon("award_star_add", 4, 7), SilkIcon("award_star_bronze_1", 2, 10),
  SilkIcon("award_star_bronze_2", 3, 9), SilkIcon("award_star_bronze_3", 4, 8), SilkIcon("award_star_delete", 5, 7), SilkIcon("award_star_gold_1", 0, 12), SilkIcon("award_star_gold_2", 1, 11), SilkIcon("award_star_gold_3", 6, 7), SilkIcon("award_star_silver_1", 0, 13),
  SilkIcon("award_star_silver_2", 1, 12), SilkIcon("award_star_silver_3", 2, 11), SilkIcon("basket", 4, 9), SilkIcon("basket_add", 5, 8), SilkIcon("basket_delete", 3, 10), SilkIcon("basket_edit", 5, 9), SilkIcon("basket_error", 6, 8), SilkIcon("basket_go", 3, 11),
  SilkIcon("basket_put", 4, 10), SilkIcon("basket_remove", 2, 12), SilkIcon("bell", 1, 13), SilkIcon("bell_add", 0, 14), SilkIcon("bell_delete", 2, 13), SilkIcon("bell_error", 1, 14), SilkIcon("bell_go", 4, 11), SilkIcon("bell_link", 5, 10), SilkIcon("bin", 3, 12),
  SilkIcon("bin_closed", 6, 9), SilkIcon("bin_empty", 5, 11), SilkIcon("bomb", 6, 10), SilkIcon("book", 3, 13), SilkIcon("book_add", 4, 12), SilkIcon("book_addresses", 2, 14), SilkIcon("book_delete", 4, 13), SilkIcon("book_edit", 5, 12), SilkIcon("book_error", 3, 14),
  SilkIcon("book_go", 6, 11), SilkIcon("book_key", 4, 14), SilkIcon("book_link", 6, 12), SilkIcon("book_next", 5, 13), SilkIcon("book_open", 5, 14), SilkIcon("book_previous", 6, 13), SilkIcon("box", 6, 14), SilkIcon("brick", 7, 0), SilkIcon("brick_add", 7, 1),
  SilkIcon("brick_delete", 8, 0), SilkIcon("brick_edit", 7, 2), SilkIcon("brick_error", 8, 1), SilkIcon("brick_go", 9, 0), SilkIcon("brick_link", 7, 3), SilkIcon("bricks", 8, 2), SilkIcon("briefcase", 9, 1), SilkIcon("bug", 10, 0), SilkIcon("bug_add", 7, 4), SilkIcon("bug_delete", 8, 3),
  SilkIcon("bug_edit", 9, 2), SilkIcon("bug_error", 10, 1), SilkIcon("bug_go", 11, 0), SilkIcon("bug_link", 9, 3), SilkIcon("building", 10, 2), SilkIcon("building_add", 11, 1), SilkIcon("building_delete", 12, 0), SilkIcon("building_edit", 8, 4), SilkIcon("building_error", 7, 5),
  SilkIcon("building_go", 9, 4), SilkIcon("building_key", 10, 3), SilkIcon("building_link", 11, 2), SilkIcon("bullet_add", 12, 1), SilkIcon("bullet_arrow_bottom", 13, 0), SilkIcon("bullet_arrow_down", 8, 5), SilkIcon("bullet_arrow_top", 7, 6), SilkIcon("bullet_arrow_up", 13, 1),
  SilkIcon("bullet_black", 14, 0), SilkIcon("bullet_blue", 9, 5), SilkIcon("bullet_delete", 7, 7), SilkIcon("bullet_disk", 8, 6), SilkIcon("bullet_error", 10, 4), SilkIcon("bullet_feed", 11, 3), SilkIcon("bullet_go", 12, 2), SilkIcon("bullet_green", 13, 2), SilkIcon("bullet_key", 14, 1),
  SilkIcon("bullet_orange", 10, 5), SilkIcon("bullet_picture", 7, 8), SilkIcon("bullet_pink", 8, 7), SilkIcon("bullet_purple", 9, 6), SilkIcon("bullet_red", 11, 4), SilkIcon("bullet_star", 12, 3), SilkIcon("bullet_toggle_minus", 9, 7), SilkIcon("bullet_toggle_plus", 10, 6),
  SilkIcon("bullet_white", 12, 4), SilkIcon("bullet_wrench", 13, 3), SilkIcon("bullet_yellow", 11, 5), SilkIcon("cake", 14, 2), SilkIcon("calculator", 7, 9), SilkIcon("calculator_add", 8, 8), SilkIcon("calculator_delete", 9, 8), SilkIcon("calculator_edit", 10, 7),
  SilkIcon("calculator_error", 11, 6), SilkIcon("calculator_link", 13, 4), SilkIcon("calendar", 14, 3), SilkIcon("calendar_add", 12, 5), SilkIcon("calendar_delete", 7, 10), SilkIcon("calendar_edit", 8, 9), SilkIcon("calendar_link", 14, 4), SilkIcon("calendar_view_day", 12, 6),
  SilkIcon("calendar_view_month", 13, 5), SilkIcon("calendar_view_week", 7, 11), SilkIcon("camera", 8, 10), SilkIcon("camera_add", 9, 9), SilkIcon("camera_delete", 10, 8), SilkIcon("camera_edit", 11, 7), SilkIcon("camera_error", 11, 8), SilkIcon("camera_go", 12, 7),
  SilkIcon("camera_link", 13, 6), SilkIcon("camera_small", 14, 5), SilkIcon("cancel", 7, 12), SilkIcon("car", 8, 11), SilkIcon("car_add", 9, 10), SilkIcon("car_delete", 10, 9), SilkIcon("cart", 7, 13), SilkIcon("cart_add", 8, 12), SilkIcon("cart_delete", 9, 11),
  SilkIcon("cart_edit", 10, 10), SilkIcon("cart_error", 11, 9), SilkIcon("cart_go", 12, 8), SilkIcon("cart_put", 13, 7), SilkIcon("cart_remove", 14, 6), SilkIcon("cd", 12, 9), SilkIcon("cd_add", 13, 8), SilkIcon("cd_burn", 14, 7), SilkIcon("cd_delete", 9, 12), SilkIcon("cd_edit", 10, 11),
  SilkIcon("cd_eject", 11, 10), SilkIcon("cd_go", 8, 13), SilkIcon("chart_bar", 7, 14), SilkIcon("chart_bar_add", 9, 13), SilkIcon("chart_bar_delete", 8, 14), SilkIcon("chart_bar_edit", 10, 12), SilkIcon("chart_bar_error", 11, 11), SilkIcon("chart_bar_link", 12, 10),
  SilkIcon("chart_curve", 14, 8), SilkIcon("chart_curve_add", 13, 9), SilkIcon("chart_curve_delete", 14, 9), SilkIcon("chart_curve_edit", 12, 11), SilkIcon("chart_curve_error", 13, 10), SilkIcon("chart_curve_go", 10, 13), SilkIcon("chart_curve_link", 11, 12), SilkIcon("chart_line", 9, 14),
  SilkIcon("chart_line_add", 11, 13), SilkIcon("chart_line_delete", 12, 12), SilkIcon("chart_line_edit", 10, 14), SilkIcon("chart_line_error", 13, 11), SilkIcon("chart_line_link", 14, 10), SilkIcon("chart_organisation", 14, 11), SilkIcon("chart_organisation_add", 11, 14),
  SilkIcon("chart_organisation_delete", 13, 12), SilkIcon("chart_pie", 12, 13), SilkIcon("chart_pie_add", 13, 13), SilkIcon("chart_pie_delete", 14, 12), SilkIcon("chart_pie_edit", 12, 14), SilkIcon("chart_pie_error", 13, 14), SilkIcon("chart_pie_link", 14, 13), SilkIcon("clock", 14, 14),
  SilkIcon("clock_add", 0, 15), SilkIcon("clock_delete", 1, 15), SilkIcon("clock_edit", 0, 16), SilkIcon("clock_error", 2, 15), SilkIcon("clock_go", 1, 16), SilkIcon("clock_link", 0, 17), SilkIcon("clock_pause", 2, 16), SilkIcon("clock_play", 3, 15), SilkIcon("clock_red", 1, 17),
  SilkIcon("clock_stop", 0, 18), SilkIcon("cog", 3, 16), SilkIcon("cog_add", 4, 15), SilkIcon("cog_delete", 1, 18), SilkIcon("cog_edit", 2, 17), SilkIcon("cog_error", 0, 19), SilkIcon("cog_go", 4, 16), SilkIcon("coins", 5, 15), SilkIcon("coins_add", 1, 19), SilkIcon("coins_delete", 2, 18),
  SilkIcon("color_swatch", 3, 17), SilkIcon("color_wheel", 0, 20), SilkIcon("comment", 3, 18), SilkIcon("comment_add", 4, 17), SilkIcon("comment_delete", 0, 21), SilkIcon("comment_edit", 1, 20), SilkIcon("comments", 5, 16), SilkIcon("comments_add", 6, 15),
  SilkIcon("comments_delete", 2, 19), SilkIcon("compress", 0, 22), SilkIcon("computer", 1, 21), SilkIcon("computer_add", 2, 20), SilkIcon("computer_delete", 5, 17), SilkIcon("computer_edit", 6, 16), SilkIcon("computer_error", 7, 15), SilkIcon("computer_go", 3, 19),
  SilkIcon("computer_key", 4, 18), SilkIcon("computer_link", 0, 23), SilkIcon("connect", 1, 22), SilkIcon("contrast", 2, 21), SilkIcon("contrast_decrease", 3, 20), SilkIcon("contrast_high", 5, 18), SilkIcon("contrast_increase", 6, 17), SilkIcon("contrast_low", 7, 16),
  SilkIcon("control_eject", 8, 15), SilkIcon("control_eject_blue", 4, 19), SilkIcon("control_end", 4, 20), SilkIcon("control_end_blue", 5, 19), SilkIcon("control_equalizer", 6, 18), SilkIcon("control_equalizer_blue", 7, 17), SilkIcon("control_fastforward", 8, 16),
  SilkIcon("control_fastforward_blue", 9, 15), SilkIcon("control_pause", 0, 24), SilkIcon("control_pause_blue", 1, 23), SilkIcon("control_play", 2, 22), SilkIcon("control_play_blue", 3, 21), SilkIcon("control_repeat", 2, 23), SilkIcon("control_repeat_blue", 3, 22),
  SilkIcon("control_rewind", 4, 21), SilkIcon("control_rewind_blue", 10, 15), SilkIcon("control_start", 5, 20), SilkIcon("control_start_blue", 6, 19), SilkIcon("control_stop", 7, 18), SilkIcon("control_stop_blue", 8, 17), SilkIcon("controller", 9, 16), SilkIcon("controller_add", 0, 25),
  SilkIcon("controller_delete", 1, 24), SilkIcon("controller_error", 4, 22), SilkIcon("creditcards", 5, 21), SilkIcon("cross", 10, 16), SilkIcon("css", 11, 15), SilkIcon("css_add", 6, 20), SilkIcon("css_delete", 7, 19), SilkIcon("css_go", 8, 18), SilkIcon("css_valid", 9, 17),
  SilkIcon("cup", 0, 26), SilkIcon("cup_add", 1, 25), SilkIcon("cup_delete", 2, 24), SilkIcon("cup_edit", 3, 23), SilkIcon("cup_error", 7, 20), SilkIcon("cup_go", 8, 19), SilkIcon("cup_key", 9, 18), SilkIcon("cup_link", 10, 17), SilkIcon("cursor", 0, 27), SilkIcon("cut", 1, 26),
  SilkIcon("cut_red", 2, 25), SilkIcon("database", 3, 24), SilkIcon("database_add", 4, 23), SilkIcon("database_connect", 5, 22), SilkIcon("database_delete", 6, 21), SilkIcon("database_edit", 11, 16), SilkIcon("database_error", 12, 15), SilkIcon("database_gear", 4, 24),
  SilkIcon("database_go", 5, 23), SilkIcon("database_key", 6, 22), SilkIcon("database_lightning", 7, 21), SilkIcon("database_link", 11, 17), SilkIcon("database_refresh", 12, 16), SilkIcon("database_save", 13, 15), SilkIcon("database_table", 10, 18), SilkIcon("date", 0, 28),
  SilkIcon("date_add", 1, 27), SilkIcon("date_delete", 2, 26), SilkIcon("date_edit", 3, 25), SilkIcon("date_error", 8, 20), SilkIcon("date_go", 9, 19), SilkIcon("date_link", 2, 27), SilkIcon("date_magnify", 3, 26), SilkIcon("date_next", 4, 25), SilkIcon("date_previous", 8, 21),
  SilkIcon("delete", 9, 20), SilkIcon("disconnect", 10, 19), SilkIcon("disk", 12, 17), SilkIcon("disk_multiple", 13, 16), SilkIcon("door", 14, 15), SilkIcon("door_in", 11, 18), SilkIcon("door_open", 0, 29), SilkIcon("door_out", 1, 28), SilkIcon("drink", 6, 23),
  SilkIcon("drink_empty", 7, 22), SilkIcon("drive", 5, 24), SilkIcon("drive_add", 1, 29), SilkIcon("drive_burn", 2, 28), SilkIcon("drive_cd", 6, 24), SilkIcon("drive_cd_empty", 7, 23), SilkIcon("drive_delete", 8, 22), SilkIcon("drive_disk", 5, 25), SilkIcon("drive_edit", 11, 19),
  SilkIcon("drive_error", 12, 18), SilkIcon("drive_go", 13, 17), SilkIcon("drive_key", 14, 16), SilkIcon("drive_link", 9, 21), SilkIcon("drive_magnify", 10, 20), SilkIcon("drive_network", 4, 26), SilkIcon("drive_rename", 3, 27), SilkIcon("drive_user", 11, 20), SilkIcon("drive_web", 4, 27),
  SilkIcon("dvd", 5, 26), SilkIcon("dvd_add", 3, 28), SilkIcon("dvd_delete", 6, 25), SilkIcon("dvd_edit", 12, 19), SilkIcon("dvd_error", 13, 18), SilkIcon("dvd_go", 14, 17), SilkIcon("dvd_key", 9, 22), SilkIcon("dvd_link", 10, 21), SilkIcon("email", 7, 24), SilkIcon("email_add", 8, 23),
  SilkIcon("email_attach", 2, 29), SilkIcon("email_delete", 3, 29), SilkIcon("email_edit", 13, 19), SilkIcon("email_error", 14, 18), SilkIcon("email_go", 9, 23), SilkIcon("email_link", 10, 22), SilkIcon("email_open", 11, 21), SilkIcon("email_open_image", 7, 25),
  SilkIcon("emoticon_evilgrin", 8, 24), SilkIcon("emoticon_grin", 4, 28), SilkIcon("emoticon_happy", 6, 26), SilkIcon("emoticon_smile", 12, 20), SilkIcon("emoticon_surprised", 5, 27), SilkIcon("emoticon_tongue", 7, 26), SilkIcon("emoticon_unhappy", 8, 25), SilkIcon("emoticon_waii", 9, 24),
  SilkIcon("emoticon_wink", 4, 29), SilkIcon("error", 5, 28), SilkIcon("error_add", 6, 27), SilkIcon("error_delete", 12, 21), SilkIcon("error_go", 13, 20), SilkIcon("exclamation", 10, 23), SilkIcon("eye", 11, 22), SilkIcon("feed", 14, 19), SilkIcon("feed_add", 13, 21),
  SilkIcon("feed_delete", 14, 20), SilkIcon("feed_disk", 10, 24), SilkIcon("feed_edit", 11, 23), SilkIcon("feed_error", 12, 22), SilkIcon("feed_go", 5, 29), SilkIcon("feed_key", 6, 28), SilkIcon("feed_link", 7, 27), SilkIcon("feed_magnify", 9, 25), SilkIcon("female", 8, 26),
  SilkIcon("film", 7, 28), SilkIcon("film_add", 8, 27), SilkIcon("film_delete", 9, 26), SilkIcon("film_edit", 10, 25), SilkIcon("film_error", 12, 23), SilkIcon("film_go", 13, 22), SilkIcon("film_key", 6, 29), SilkIcon("film_link", 11, 24), SilkIcon("film_save", 14, 21),
  SilkIcon("find", 7, 29), SilkIcon("flag_blue", 11, 25), SilkIcon("flag_green", 12, 24), SilkIcon("flag_orange", 14, 22), SilkIcon("flag_pink", 13, 23), SilkIcon("flag_purple", 9, 27), SilkIcon("flag_red", 10, 26), SilkIcon("flag_yellow", 8, 28), SilkIcon("folder", 8, 29),
  SilkIcon("folder_add", 9, 28), SilkIcon("folder_bell", 13, 24), SilkIcon("folder_brick", 14, 23), SilkIcon("folder_bug", 10, 27), SilkIcon("folder_camera", 11, 26), SilkIcon("folder_database", 12, 25), SilkIcon("folder_delete", 10, 28), SilkIcon("folder_edit", 11, 27),
  SilkIcon("folder_error", 12, 26), SilkIcon("folder_explore", 13, 25), SilkIcon("folder_feed", 14, 24), SilkIcon("folder_find", 9, 29), SilkIcon("folder_go", 14, 25), SilkIcon("folder_heart", 10, 29), SilkIcon("folder_image", 12, 27), SilkIcon("folder_key", 13, 26),
  SilkIcon("folder_lightbulb", 11, 28), SilkIcon("folder_link", 14, 26), SilkIcon("folder_magnify", 11, 29), SilkIcon("folder_page", 12, 28), SilkIcon("folder_page_white", 13, 27), SilkIcon("folder_palette", 14, 27), SilkIcon("folder_picture", 12, 29), SilkIcon("folder_star", 13, 28),
  SilkIcon("folder_table", 13, 29), SilkIcon("folder_user", 14, 28), SilkIcon("folder_wrench", 14, 29), SilkIcon("font", 15, 0), SilkIcon("font_add", 15, 1), SilkIcon("font_delete", 16, 0), SilkIcon("font_go", 16, 1), SilkIcon("group", 17, 0), SilkIcon("group_add", 15, 2),
  SilkIcon("group_delete", 15, 3), SilkIcon("group_edit", 17, 1), SilkIcon("group_error", 18, 0), SilkIcon("group_gear", 16, 2), SilkIcon("group_go", 15, 4), SilkIcon("group_key", 16, 3), SilkIcon("group_link", 18, 1), SilkIcon("heart", 19, 0), SilkIcon("heart_add", 17, 2),
  SilkIcon("heart_delete", 19, 1), SilkIcon("help", 20, 0), SilkIcon("hourglass", 18, 2), SilkIcon("hourglass_add", 17, 3), SilkIcon("hourglass_delete", 15, 5), SilkIcon("hourglass_go", 16, 4), SilkIcon("hourglass_link", 15, 6), SilkIcon("house", 20, 1), SilkIcon("house_go", 21, 0),
  SilkIcon("house_link", 19, 2), SilkIcon("html", 17, 4), SilkIcon("html_add", 18, 3), SilkIcon("html_delete", 16, 5), SilkIcon("html_go", 16, 6), SilkIcon("html_valid", 20, 2), SilkIcon("image", 21, 1), SilkIcon("image_add", 22, 0), SilkIcon("image_delete", 19, 3),
  SilkIcon("image_edit", 18, 4), SilkIcon("image_link", 17, 5), SilkIcon("images", 15, 7), SilkIcon("information", 16, 7), SilkIcon("ipod", 17, 6), SilkIcon("ipod_cast", 21, 2), SilkIcon("ipod_cast_add", 22, 1), SilkIcon("ipod_cast_delete", 23, 0), SilkIcon("ipod_sound", 19, 4),
  SilkIcon("joystick", 20, 3), SilkIcon("joystick_add", 18, 5), SilkIcon("joystick_delete", 15, 8), SilkIcon("joystick_error", 16, 8), SilkIcon("key", 17, 7), SilkIcon("key_add", 18, 6), SilkIcon("key_delete", 21, 3), SilkIcon("key_go", 22, 2), SilkIcon("keyboard", 23, 1),
  SilkIcon("keyboard_add", 24, 0), SilkIcon("keyboard_delete", 20, 4), SilkIcon("keyboard_magnify", 19, 5), SilkIcon("layers", 15, 9), SilkIcon("layout", 23, 2), SilkIcon("layout_add", 24, 1), SilkIcon("layout_content", 25, 0), SilkIcon("layout_delete", 20, 5),
  SilkIcon("layout_edit", 21, 4), SilkIcon("layout_error", 15, 10), SilkIcon("layout_header", 16, 9), SilkIcon("layout_link", 19, 6), SilkIcon("layout_sidebar", 17, 8), SilkIcon("lightbulb", 18, 7), SilkIcon("lightbulb_add", 22, 3), SilkIcon("lightbulb_delete", 24, 2),
  SilkIcon("lightbulb_off", 25, 1), SilkIcon("lightning", 26, 0), SilkIcon("lightning_add", 20, 6), SilkIcon("lightning_delete", 21, 5), SilkIcon("lightning_go", 22, 4), SilkIcon("link", 15, 11), SilkIcon("link_add", 16, 10), SilkIcon("link_break", 17, 9), SilkIcon("link_delete", 19, 7),
  SilkIcon("link_edit", 18, 8), SilkIcon("link_error", 23, 3), SilkIcon("link_go", 19, 8), SilkIcon("lock", 20, 7), SilkIcon("lock_add", 18, 9), SilkIcon("lock_break", 24, 3), SilkIcon("lock_delete", 23, 4), SilkIcon("lock_edit", 25, 2), SilkIcon("lock_go", 26, 1),
  SilkIcon("lock_open", 27, 0), SilkIcon("lorry", 21, 6), SilkIcon("lorry_add", 22, 5), SilkIcon("lorry_delete", 15, 12), SilkIcon("lorry_error", 16, 11), SilkIcon("lorry_flatbed", 17, 10), SilkIcon("lorry_go", 20, 8), SilkIcon("lorry_link", 21, 7), SilkIcon("magifier_zoom_out", 18, 10),
  SilkIcon("magnifier", 19, 9), SilkIcon("magnifier_zoom_in", 25, 3), SilkIcon("male", 23, 5), SilkIcon("map", 24, 4), SilkIcon("map_add", 26, 2), SilkIcon("map_delete", 27, 1), SilkIcon("map_edit", 28, 0), SilkIcon("map_go", 22, 6), SilkIcon("map_magnify", 15, 13),
  SilkIcon("medal_bronze_1", 16, 12), SilkIcon("medal_bronze_2", 17, 11), SilkIcon("medal_bronze_3", 15, 14), SilkIcon("medal_bronze_add", 20, 9), SilkIcon("medal_bronze_delete", 21, 8), SilkIcon("medal_gold_1", 22, 7), SilkIcon("medal_gold_2", 19, 10), SilkIcon("medal_gold_3", 25, 4),
  SilkIcon("medal_gold_add", 26, 3), SilkIcon("medal_gold_delete", 23, 6), SilkIcon("medal_silver_1", 24, 5), SilkIcon("medal_silver_2", 27, 2), SilkIcon("medal_silver_3", 28, 1), SilkIcon("medal_silver_add", 16, 13), SilkIcon("medal_silver_delete", 17, 12), SilkIcon("money", 18, 11),
  SilkIcon("money_add", 22, 8), SilkIcon("money_delete", 23, 7), SilkIcon("money_dollar", 20, 10), SilkIcon("money_euro", 25, 5), SilkIcon("money_pound", 26, 4), SilkIcon("money_yen", 27, 3), SilkIcon("monitor", 24, 6), SilkIcon("monitor_add", 28, 2), SilkIcon("monitor_delete", 16, 14),
  SilkIcon("monitor_edit", 17, 13), SilkIcon("monitor_error", 18, 12), SilkIcon("monitor_go", 19, 11), SilkIcon("monitor_lightning", 15, 15), SilkIcon("monitor_link", 21, 9), SilkIcon("mouse", 22, 9), SilkIcon("mouse_add", 15, 16), SilkIcon("mouse_delete", 23, 8),
  SilkIcon("mouse_error", 24, 7), SilkIcon("music", 20, 11), SilkIcon("new", 21, 10), SilkIcon("newspaper", 25, 6), SilkIcon("newspaper_add", 26, 5), SilkIcon("newspaper_delete", 27, 4), SilkIcon("newspaper_go", 28, 3), SilkIcon("newspaper_link", 16, 15), SilkIcon("note", 17, 14),
  SilkIcon("note_add", 18, 13), SilkIcon("note_delete", 19, 12), SilkIcon("note_edit", 25, 7), SilkIcon("note_error", 20, 12), SilkIcon("note_go", 21, 11), SilkIcon("overlays", 22, 10), SilkIcon("package", 26, 6), SilkIcon("package_add", 27, 5), SilkIcon("package_delete", 28, 4),
  SilkIcon("package_go", 16, 16), SilkIcon("package_green", 17, 15), SilkIcon("package_link", 18, 14), SilkIcon("page", 19, 13), SilkIcon("page_add", 15, 17), SilkIcon("page_attach", 23, 9), SilkIcon("page_code", 24, 8), SilkIcon("page_copy", 24, 9), SilkIcon("page_delete", 25, 8),
  SilkIcon("page_edit", 22, 11), SilkIcon("page_error", 23, 10), SilkIcon("page_excel", 26, 7), SilkIcon("page_find", 27, 6), SilkIcon("page_gear", 28, 5), SilkIcon("page_go", 16, 17), SilkIcon("page_green", 17, 16), SilkIcon("page_key", 18, 15), SilkIcon("page_lightning", 19, 14),
  SilkIcon("page_link", 20, 13), SilkIcon("page_paintbrush", 15, 18), SilkIcon("page_paste", 21, 12), SilkIcon("page_red", 24, 10), SilkIcon("page_refresh", 25, 9), SilkIcon("page_save", 26, 8), SilkIcon("page_white", 22, 12), SilkIcon("page_white_acrobat", 23, 11),
  SilkIcon("page_white_actionscript", 27, 7), SilkIcon("page_white_add", 28, 6), SilkIcon("page_white_c", 16, 18), SilkIcon("page_white_camera", 17, 17), SilkIcon("page_white_cd", 18, 16), SilkIcon("page_white_code", 19, 15), SilkIcon("page_white_code_red", 20, 14),
  SilkIcon("page_white_coldfusion", 21, 13), SilkIcon("page_white_compressed", 15, 19), SilkIcon("page_white_copy", 17, 18), SilkIcon("page_white_cplusplus", 18, 17), SilkIcon("page_white_csharp", 19, 16), SilkIcon("page_white_cup", 20, 15), SilkIcon("page_white_database", 21, 14),
  SilkIcon("page_white_delete", 22, 13), SilkIcon("page_white_dvd", 15, 20), SilkIcon("page_white_edit", 16, 19), SilkIcon("page_white_error", 24, 11), SilkIcon("page_white_excel", 25, 10), SilkIcon("page_white_find", 26, 9), SilkIcon("page_white_flash", 27, 8),
  SilkIcon("page_white_freehand", 23, 12), SilkIcon("page_white_gear", 28, 7), SilkIcon("page_white_get", 21, 15), SilkIcon("page_white_go", 22, 14), SilkIcon("page_white_h", 23, 13), SilkIcon("page_white_horizontal", 15, 21), SilkIcon("page_white_key", 16, 20),
  SilkIcon("page_white_lightning", 17, 19), SilkIcon("page_white_link", 25, 11), SilkIcon("page_white_magnify", 26, 10), SilkIcon("page_white_medal", 27, 9), SilkIcon("page_white_office", 28, 8), SilkIcon("page_white_paint", 24, 12), SilkIcon("page_white_paintbrush", 18, 18),
  SilkIcon("page_white_paste", 19, 17), SilkIcon("page_white_php", 20, 16), SilkIcon("page_white_picture", 15, 22), SilkIcon("page_white_powerpoint", 21, 16), SilkIcon("page_white_put", 22, 15), SilkIcon("page_white_ruby", 23, 14), SilkIcon("page_white_stack", 24, 13),
  SilkIcon("page_white_star", 16, 21), SilkIcon("page_white_swoosh", 17, 20), SilkIcon("page_white_text", 18, 19), SilkIcon("page_white_text_width", 25, 12), SilkIcon("page_white_tux", 26, 11), SilkIcon("page_white_vector", 27, 10), SilkIcon("page_white_visualstudio", 28, 9),
  SilkIcon("page_white_width", 19, 18), SilkIcon("page_white_word", 20, 17), SilkIcon("page_white_world", 24, 14), SilkIcon("page_white_wrench", 25, 13), SilkIcon("page_white_zip", 16, 22), SilkIcon("page_word", 17, 21), SilkIcon("page_world", 18, 20), SilkIcon("paintbrush", 19, 19),
  SilkIcon("paintcan", 26, 12), SilkIcon("palette", 27, 11), SilkIcon("paste_plain", 28, 10), SilkIcon("paste_word", 20, 18), SilkIcon("pencil", 21, 17), SilkIcon("pencil_add", 15, 23), SilkIcon("pencil_delete", 22, 16), SilkIcon("pencil_go", 23, 15), SilkIcon("phone", 20, 19),
  SilkIcon("phone_add", 21, 18), SilkIcon("phone_delete", 22, 17), SilkIcon("phone_sound", 15, 24), SilkIcon("photo", 16, 23), SilkIcon("photo_add", 23, 16), SilkIcon("photo_delete", 24, 15), SilkIcon("photo_link", 25, 14), SilkIcon("photos", 26, 13), SilkIcon("picture", 17, 22),
  SilkIcon("picture_add", 18, 21), SilkIcon("picture_delete", 19, 20), SilkIcon("picture_edit", 27, 12), SilkIcon("picture_empty", 28, 11), SilkIcon("picture_error", 28, 12), SilkIcon("picture_go", 21, 19), SilkIcon("picture_key", 22, 18), SilkIcon("picture_link", 23, 17),
  SilkIcon("picture_save", 15, 25), SilkIcon("pictures", 16, 24), SilkIcon("pilcrow", 17, 23), SilkIcon("pill", 24, 16), SilkIcon("pill_add", 25, 15), SilkIcon("pill_delete", 26, 14), SilkIcon("pill_go", 27, 13), SilkIcon("plugin", 18, 22), SilkIcon("plugin_add", 19, 21),
  SilkIcon("plugin_delete", 20, 20), SilkIcon("plugin_disabled", 16, 25), SilkIcon("plugin_edit", 17, 24), SilkIcon("plugin_error", 18, 23), SilkIcon("plugin_go", 24, 17), SilkIcon("plugin_link", 25, 16), SilkIcon("printer", 26, 15), SilkIcon("printer_add", 27, 14),
  SilkIcon("printer_delete", 28, 13), SilkIcon("printer_empty", 19, 22), SilkIcon("printer_error", 20, 21), SilkIcon("rainbow", 21, 20), SilkIcon("report", 22, 19), SilkIcon("report_add", 23, 18), SilkIcon("report_delete", 15, 26), SilkIcon("report_disk", 28, 14),
  SilkIcon("report_edit", 19, 23), SilkIcon("report_go", 20, 22), SilkIcon("report_key", 21, 21), SilkIcon("report_link", 22, 20), SilkIcon("report_magnify", 23, 19), SilkIcon("report_picture", 24, 18), SilkIcon("report_user", 15, 27), SilkIcon("report_word", 16, 26),
  SilkIcon("resultset_first", 18, 24), SilkIcon("resultset_last", 25, 17), SilkIcon("resultset_next", 26, 16), SilkIcon("resultset_previous", 27, 15), SilkIcon("rosette", 17, 25), SilkIcon("rss", 19, 24), SilkIcon("rss_add", 25, 18), SilkIcon("rss_delete", 26, 17),
  SilkIcon("rss_go", 27, 16), SilkIcon("rss_valid", 28, 15), SilkIcon("ruby", 17, 26), SilkIcon("ruby_add", 18, 25), SilkIcon("ruby_delete", 22, 21), SilkIcon("ruby_gear", 23, 20), SilkIcon("ruby_get", 24, 19), SilkIcon("ruby_go", 15, 28), SilkIcon("ruby_key", 16, 27),
  SilkIcon("ruby_link", 20, 23), SilkIcon("ruby_put", 21, 22), SilkIcon("script", 16, 28), SilkIcon("script_add", 17, 27), SilkIcon("script_code", 20, 24), SilkIcon("script_code_red", 21, 23), SilkIcon("script_delete", 22, 22), SilkIcon("script_edit", 18, 26),
  SilkIcon("script_error", 19, 25), SilkIcon("script_gear", 23, 21), SilkIcon("script_go", 24, 20), SilkIcon("script_key", 25, 19), SilkIcon("script_lightning", 15, 29), SilkIcon("script_link", 26, 18), SilkIcon("script_palette", 27, 17), SilkIcon("script_save", 28, 16),
  SilkIcon("server", 28, 17), SilkIcon("server_add", 20, 25), SilkIcon("server_chart", 23, 22), SilkIcon("server_compressed", 24, 21), SilkIcon("server_connect", 25, 20), SilkIcon("server_database", 26, 19), SilkIcon("server_delete", 16, 29), SilkIcon("server_edit", 27, 18),
  SilkIcon("server_error", 22, 23), SilkIcon("server_go", 18, 27), SilkIcon("server_key", 19, 26), SilkIcon("server_lightning", 21, 24), SilkIcon("server_link", 17, 28), SilkIcon("server_uncompressed", 18, 28), SilkIcon("shading", 17, 29), SilkIcon("shape_align_bottom", 27, 19),
  SilkIcon("shape_align_center", 28, 18), SilkIcon("shape_align_left", 22, 24), SilkIcon("shape_align_middle", 23, 23), SilkIcon("shape_align_right", 19, 27), SilkIcon("shape_align_top", 20, 26), SilkIcon("shape_flip_horizontal", 21, 25), SilkIcon("shape_flip_vertical", 25, 21),
  SilkIcon("shape_group", 26, 20), SilkIcon("shape_handles", 24, 22), SilkIcon("shape_move_back", 20, 27), SilkIcon("shape_move_backwards", 21, 26), SilkIcon("shape_move_forwards", 22, 25), SilkIcon("shape_move_front", 25, 22), SilkIcon("shape_rotate_anticlockwise", 26, 21),
  SilkIcon("shape_rotate_clockwise", 27, 20), SilkIcon("shape_square", 24, 23), SilkIcon("shape_square_add", 23, 24), SilkIcon("shape_square_delete", 19, 28), SilkIcon("shape_square_edit", 28, 19), SilkIcon("shape_square_error", 18, 29), SilkIcon("shape_square_go", 25, 23),
  SilkIcon("shape_square_key", 23, 25), SilkIcon("shape_square_link", 24, 24), SilkIcon("shape_ungroup", 19, 29), SilkIcon("shield", 20, 28), SilkIcon("shield_add", 28, 20), SilkIcon("shield_delete", 26, 22), SilkIcon("shield_go", 27, 21), SilkIcon("sitemap", 22, 26),
  SilkIcon("sitemap_color", 21, 27), SilkIcon("sound", 27, 22), SilkIcon("sound_add", 28, 21), SilkIcon("sound_delete", 22, 27), SilkIcon("sound_low", 23, 26), SilkIcon("sound_mute", 21, 28), SilkIcon("sound_none", 20, 29), SilkIcon("spellcheck", 26, 23), SilkIcon("sport_8ball", 24, 25),
  SilkIcon("sport_basketball", 25, 24), SilkIcon("sport_football", 24, 26), SilkIcon("sport_golf", 25, 25), SilkIcon("sport_raquet", 26, 24), SilkIcon("sport_shuttlecock", 21, 29), SilkIcon("sport_soccer", 22, 28), SilkIcon("sport_tennis", 27, 23), SilkIcon("star", 23, 27),
  SilkIcon("status_away", 28, 22), SilkIcon("status_busy", 28, 23), SilkIcon("status_offline", 22, 29), SilkIcon("status_online", 23, 28), SilkIcon("stop", 27, 24), SilkIcon("style", 24, 27), SilkIcon("style_add", 26, 25), SilkIcon("style_delete", 25, 26), SilkIcon("style_edit", 28, 24),
  SilkIcon("style_go", 24, 28), SilkIcon("sum", 25, 27), SilkIcon("tab", 26, 26), SilkIcon("tab_add", 27, 25), SilkIcon("tab_delete", 23, 29), SilkIcon("tab_edit", 27, 26), SilkIcon("tab_go", 28, 25), SilkIcon("table", 24, 29), SilkIcon("table_add", 25, 28),
  SilkIcon("table_delete", 26, 27), SilkIcon("table_edit", 26, 28), SilkIcon("table_error", 27, 27), SilkIcon("table_gear", 25, 29), SilkIcon("table_go", 28, 26), SilkIcon("table_key", 26, 29), SilkIcon("table_lightning", 28, 27), SilkIcon("table_link", 27, 28),
  SilkIcon("table_multiple", 27, 29), SilkIcon("table_refresh", 28, 28), SilkIcon("table_relationship", 28, 29), SilkIcon("table_row_delete", 0, 30), SilkIcon("table_row_insert", 0, 31), SilkIcon("table_save", 1, 30), SilkIcon("table_sort", 2, 30), SilkIcon("tag", 1, 31),
  SilkIcon("tag_blue", 0, 32), SilkIcon("tag_blue_add", 3, 30), SilkIcon("tag_blue_delete", 1, 32), SilkIcon("tag_blue_edit", 2, 31), SilkIcon("tag_green", 0, 33), SilkIcon("tag_orange", 3, 31), SilkIcon("tag_pink", 4, 30), SilkIcon("tag_purple", 1, 33), SilkIcon("tag_red", 2, 32),
  SilkIcon("tag_yellow", 0, 34), SilkIcon("telephone", 1, 34), SilkIcon("telephone_add", 5, 30), SilkIcon("telephone_delete", 3, 32), SilkIcon("telephone_edit", 4, 31), SilkIcon("telephone_error", 2, 33), SilkIcon("telephone_go", 0, 35), SilkIcon("telephone_key", 6, 30),
  SilkIcon("telephone_link", 1, 35), SilkIcon("television", 2, 34), SilkIcon("television_add", 5, 31), SilkIcon("television_delete", 4, 32), SilkIcon("text_align_center", 3, 33), SilkIcon("text_align_justify", 0, 36), SilkIcon("text_align_left", 3, 34), SilkIcon("text_align_right", 5, 32),
  SilkIcon("text_allcaps", 6, 31), SilkIcon("text_bold", 4, 33), SilkIcon("text_columns", 1, 36), SilkIcon("text_dropcaps", 0, 37), SilkIcon("text_heading_1", 7, 30), SilkIcon("text_heading_2", 2, 35), SilkIcon("text_heading_3", 6, 32), SilkIcon("text_heading_4", 7, 31),
  SilkIcon("text_heading_5", 4, 34), SilkIcon("text_heading_6", 5, 33), SilkIcon("text_horizontalrule", 1, 37), SilkIcon("text_indent", 2, 36), SilkIcon("text_indent_remove", 8, 30), SilkIcon("text_italic", 3, 35), SilkIcon("text_kerning", 0, 38), SilkIcon("text_letter_omega", 6, 33),
  SilkIcon("text_letterspacing", 7, 32), SilkIcon("text_linespacing", 8, 31), SilkIcon("text_list_bullets", 4, 35), SilkIcon("text_list_numbers", 5, 34), SilkIcon("text_lowercase", 1, 38), SilkIcon("text_padding_bottom", 2, 37), SilkIcon("text_padding_left", 3, 36),
  SilkIcon("text_padding_right", 9, 30), SilkIcon("text_padding_top", 0, 39), SilkIcon("text_replace", 7, 33), SilkIcon("text_signature", 8, 32), SilkIcon("text_smallcaps", 9, 31), SilkIcon("text_strikethrough", 4, 36), SilkIcon("text_subscript", 5, 35),
  SilkIcon("text_superscript", 6, 34), SilkIcon("text_underline", 1, 39), SilkIcon("text_uppercase", 2, 38), SilkIcon("textfield", 3, 37), SilkIcon("textfield_add", 10, 30), SilkIcon("textfield_delete", 0, 40), SilkIcon("textfield_key", 11, 30), SilkIcon("textfield_rename", 7, 34),
  SilkIcon("thumb_down", 8, 33), SilkIcon("thumb_up", 9, 32), SilkIcon("tick", 10, 31), SilkIcon("time", 4, 37), SilkIcon("time_add", 5, 36), SilkIcon("time_delete", 6, 35), SilkIcon("time_go", 2, 39), SilkIcon("timeline_marker", 3, 38), SilkIcon("transmit", 1, 40),
  SilkIcon("transmit_add", 0, 41), SilkIcon("transmit_blue", 10, 32), SilkIcon("transmit_delete", 11, 31), SilkIcon("transmit_edit", 4, 38), SilkIcon("transmit_error", 5, 37), SilkIcon("transmit_go", 6, 36), SilkIcon("tux", 7, 35), SilkIcon("user", 2, 40), SilkIcon("user_add", 3, 39),
  SilkIcon("user_comment", 1, 41), SilkIcon("user_delete", 0, 42), SilkIcon("user_edit", 12, 30), SilkIcon("user_female", 8, 34), SilkIcon("user_go", 9, 33), SilkIcon("user_gray", 10, 33), SilkIcon("user_green", 11, 32), SilkIcon("user_orange", 12, 31), SilkIcon("user_red", 4, 39),
  SilkIcon("user_suit", 5, 38), SilkIcon("vcard", 6, 37), SilkIcon("vcard_add", 7, 36), SilkIcon("vcard_delete", 8, 35), SilkIcon("vcard_edit", 2, 41), SilkIcon("vector", 3, 40), SilkIcon("vector_add", 1, 42), SilkIcon("vector_delete", 0, 43), SilkIcon("wand", 13, 30),
  SilkIcon("weather_clouds", 9, 34), SilkIcon("weather_cloudy", 10, 34), SilkIcon("weather_lightning", 11, 33), SilkIcon("weather_rain", 12, 32), SilkIcon("weather_snow", 13, 31), SilkIcon("weather_sun", 4, 40), SilkIcon("webcam", 5, 39), SilkIcon("webcam_add", 6, 38),
  SilkIcon("webcam_delete", 7, 37), SilkIcon("webcam_error", 8, 36), SilkIcon("world", 9, 35), SilkIcon("world_add", 2, 42), SilkIcon("world_delete", 3, 41), SilkIcon("world_edit", 1, 43), SilkIcon("world_go", 0, 44), SilkIcon("world_link", 14, 30), SilkIcon("wrench", 12, 33),
  SilkIcon("wrench_orange", 13, 32), SilkIcon("xhtml", 14, 31), SilkIcon("xhtml_add", 4, 41), SilkIcon("xhtml_delete", 5, 40), SilkIcon("xhtml_go", 6, 39), SilkIcon("xhtml_valid", 7, 38), SilkIcon("zoom", 8, 37), SilkIcon("zoom_in", 9, 36), SilkIcon("zoom_out", 10, 35)};

void WISilkIcon::SetIcon(std::string icon)
{
	StringToLower(icon);
	SilkIcon *info = NULL;
	for(unsigned int i = 0; i < (sizeof(icons) / sizeof(SilkIcon)); i++) {
		if(icons[i].name == icon) {
			info = &icons[i];
			break;
		}
	}
	if(info == NULL)
		return;
	auto xIcon = info->col * 16 + info->col * 1;
	auto yIcon = info->row * 16 + info->row * 1;
	SetClipping(xIcon, yIcon, 16, 16);
}
