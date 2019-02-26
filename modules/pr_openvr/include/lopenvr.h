#ifndef __LOPENVR_H__
#define __LOPENVR_H__

struct lua_State;
namespace Lua
{
	class Interface;
	namespace openvr
	{
		void register_lua_library(Lua::Interface &l);
		namespace lib
		{
			int initialize(lua_State *l);
			int close(lua_State *l);
			//int get_tracked_device_property(lua_State *l);

			int property_error_to_string(lua_State *l);
			int init_error_to_string(lua_State *l);
			int button_id_to_string(lua_State *l);
			int controller_axis_type_to_string(lua_State *l);

			int get_tracking_system_name(lua_State *l);
			int get_model_number(lua_State *l);
			int get_serial_number(lua_State *l);
			int get_render_model_name(lua_State *l);
			int get_manufacturer_name(lua_State *l);
			int get_tracking_firmware_version(lua_State *l);
			int get_hardware_revision(lua_State *l);
			int get_all_wireless_dongle_descriptions(lua_State *l);
			int get_connected_wireless_dongle(lua_State *l);
			int get_firmware_manual_update_url(lua_State *l);
			int get_firmware_programming_target(lua_State *l);
			int get_display_mc_image_left(lua_State *l);
			int get_display_mc_image_right(lua_State *l);
			int get_display_gc_image(lua_State *l);
			int get_camera_firmware_description(lua_State *l);
			int get_attached_device_id(lua_State *l);
			int get_model_label(lua_State *l);

			int will_drift_in_yaw(lua_State *l);
			int device_is_wireless(lua_State *l);
			int device_is_charging(lua_State *l);
			int firmware_update_available(lua_State *l);
			int firmware_manual_update(lua_State *l);
			int block_server_shutdown(lua_State *l);
			int can_unify_coordinate_system_with_hmd(lua_State *l);
			int contains_proximity_sensor(lua_State *l);
			int device_provides_battery_status(lua_State *l);
			int device_can_power_off(lua_State *l);
			int has_camera(lua_State *l);
			int reports_time_since_vsync(lua_State *l);
			int is_on_desktop(lua_State *l);

			int get_device_battery_percentage(lua_State *l);
			int get_seconds_from_vsync_to_photons(lua_State *l);
			int get_display_frequency(lua_State *l);
			int get_user_ipd_meters(lua_State *l);
			int get_display_mc_offset(lua_State *l);
			int get_display_mc_scale(lua_State *l);
			int get_display_gc_black_clamp(lua_State *l);
			int get_display_gc_offset(lua_State *l);
			int get_display_gc_scale(lua_State *l);
			int get_display_gc_prescale(lua_State *l);
			int get_lens_center_left_u(lua_State *l);
			int get_lens_center_left_v(lua_State *l);
			int get_lens_center_left_uv(lua_State *l);
			int get_lens_center_right_u(lua_State *l);
			int get_lens_center_right_v(lua_State *l);
			int get_lens_center_right_uv(lua_State *l);
			int get_user_head_to_eye_depth_meters(lua_State *l);
			int get_field_of_view_left_degrees(lua_State *l);
			int get_field_of_view_right_degrees(lua_State *l);
			int get_field_of_view_top_degrees(lua_State *l);
			int get_field_of_view_bottom_degrees(lua_State *l);
			int get_tracking_range_minimum_meters(lua_State *l);
			int get_tracking_range_maximum_meters(lua_State *l);

			int get_status_display_transform(lua_State *l);
			int get_camera_to_head_transform(lua_State *l);

			int get_hardware_revision_number(lua_State *l);
			int get_firmware_version(lua_State *l);
			int get_fpga_version(lua_State *l);
			int get_vrc_version(lua_State *l);
			int get_radio_version(lua_State *l);
			int get_dongle_version(lua_State *l);
			int get_current_universe_id(lua_State *l);
			int get_previous_universe_id(lua_State *l);
			int get_display_firmware_version(lua_State *l);
			int get_camera_firmware_version(lua_State *l);
			int get_display_fpga_version(lua_State *l);
			int get_display_bootloader_version(lua_State *l);
			int get_display_hardware_version(lua_State *l);
			int get_audio_firmware_version(lua_State *l);
			int get_supported_buttons(lua_State *l);

			int get_device_class(lua_State *l);
			int get_display_mc_type(lua_State *l);
			int get_edid_vendor_id(lua_State *l);
			int get_edid_product_id(lua_State *l);
			int get_display_gc_type(lua_State *l);
			int get_camera_compatibility_mode(lua_State *l);
			int get_axis0_type(lua_State *l);
			int get_axis1_type(lua_State *l);
			int get_axis2_type(lua_State *l);
			int get_axis3_type(lua_State *l);
			int get_axis4_type(lua_State *l);

			int fade_to_color(lua_State *l);
			int fade_grid(lua_State *l);
			int show_mirror_window(lua_State *l);
			int hide_mirror_window(lua_State *l);
			int is_mirror_window_visible(lua_State *l);
			int set_hmd_view_enabled(lua_State *l);
			int is_hmd_view_enabled(lua_State *l);

			int can_render_scene(lua_State *l);
			int clear_last_submitted_frame(lua_State *l);
			int clear_skybox_override(lua_State *l);
			int compositor_bring_to_front(lua_State *l);
			int compositor_dump_images(lua_State *l);
			int compositor_go_to_back(lua_State *l);
			int force_interleaved_reprojection_on(lua_State *l);
			int force_reconnect_process(lua_State *l);
			int get_frame_time_remaining(lua_State *l);
			int is_fullscreen(lua_State *l);
			int should_app_render_with_low_resources(lua_State *l);
			int suspend_rendering(lua_State *l);
			int set_skybox_override(lua_State *l);
			int get_cumulative_stats(lua_State *l);
			int get_tracking_space(lua_State *l);
			int set_tracking_space(lua_State *l);

			int get_recommended_render_target_size(lua_State *l);
			int get_projection_matrix(lua_State *l);
			int get_projection_raw(lua_State *l);
			int compute_distortion(lua_State *l);
			int get_eye_to_head_transform(lua_State *l);
			int get_time_since_last_vsync(lua_State *l);
			int get_device_to_absolute_tracking_pose(lua_State *l);
			int compute_seconds_to_photons(lua_State *l);
			int reset_seated_zero_pose(lua_State *l);
			int get_seated_zero_pose_to_standing_absolute_tracking_pose(lua_State *l);
			int get_tracked_device_class(lua_State *l);
			int is_tracked_device_connected(lua_State *l);
			int trigger_haptic_pulse(lua_State *l);
			int get_controller_state(lua_State *l);
			int get_controller_states(lua_State *l);
			int get_controller_state_with_pose(lua_State *l);

			int get_pose_transform(lua_State *l);
		};
	};
};

#endif