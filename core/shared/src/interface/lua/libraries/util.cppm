// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.util;

export import :game.bullet_info;
export import :game.damage_info;
export import :physics.raycast;
export import :scripting.lua.classes.property;
export import :util.any;
export import :util.splash_damage_info;

export namespace Lua {
	namespace global {
		DLLNETWORK std::string get_script_path();

		DLLNETWORK EulerAngles angle_rand();
		DLLNETWORK EulerAngles create_from_string(const std::string &str);
	};
	namespace util {
		DLLNETWORK void splash_damage(lua::State *l, const pragma::util::SplashDamageInfo &splashDamageInfo);

		DLLNETWORK bool is_valid(lua::State *l);
		DLLNETWORK bool is_valid(lua::State *l, const luabind::object &o);
		DLLNETWORK bool is_valid_entity(lua::State *l);
		DLLNETWORK bool is_valid_entity(lua::State *l, const luabind::object &o);
		DLLNETWORK void remove(lua::State *l, const luabind::object &o);
		DLLNETWORK void remove(lua::State *l, const luabind::object &o, bool removeSafely);
		DLLNETWORK bool is_table(luabind::argument arg);
		DLLNETWORK bool is_table();
		DLLNETWORK std::string date_time(const std::string &format);
		DLLNETWORK std::string date_time();
		DLLNETWORK luabind::object fire_bullets(lua::State *l, pragma::game::BulletInfo &bulletInfo, bool hitReport, const std::function<void(pragma::game::DamageInfo &, pragma::physics::TraceData &, pragma::physics::TraceResult &, uint32_t &)> &f);
		DLLNETWORK luabind::object fire_bullets(lua::State *l, pragma::game::BulletInfo &bulletInfo, bool hitReport);
		DLLNETWORK luabind::object fire_bullets(lua::State *l, pragma::game::BulletInfo &bulletInfo);

		DLLNETWORK luabind::object register_class(lua::State *l, const std::string &className);
		DLLNETWORK luabind::object register_class(lua::State *l, const std::string &className, const luabind::object &oBase0);
		DLLNETWORK luabind::object register_class(lua::State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1);
		DLLNETWORK luabind::object register_class(lua::State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2);
		DLLNETWORK luabind::object register_class(lua::State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3);
		DLLNETWORK luabind::object register_class(lua::State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3, const luabind::object &oBase4);
		DLLNETWORK luabind::object register_class(lua::State *l, const luabind::table<> &t, const std::string &className);
		DLLNETWORK luabind::object register_class(lua::State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0);
		DLLNETWORK luabind::object register_class(lua::State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1);
		DLLNETWORK luabind::object register_class(lua::State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2);
		DLLNETWORK luabind::object register_class(lua::State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3);
		DLLNETWORK luabind::object register_class(lua::State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3, const luabind::object &oBase4);

		DLLNETWORK void shake_screen(lua::State *l, const Vector3 &pos, float radius, float amplitude, float frequency, float duration, float fadeIn, float fadeOut);
		DLLNETWORK void shake_screen(lua::State *l, float amplitude, float frequency, float duration, float fadeIn, float fadeOut);
		DLLNETWORK float get_faded_time_factor(float cur, float dur, float fadeIn, float fadeOut);
		DLLNETWORK float get_faded_time_factor(float cur, float dur, float fadeIn);
		DLLNETWORK float get_faded_time_factor(float cur, float dur);
		DLLNETWORK float get_scale_factor(float val, float min, float max);
		DLLNETWORK float get_scale_factor(float val, float min);

		DLLNETWORK Quat local_to_world(lua::State *l, const Quat &r0, const Quat &r1);
		DLLNETWORK Vector3 local_to_world(lua::State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v);
		DLLNETWORK void local_to_world(lua::State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v, const Quat &r);

		DLLNETWORK Quat world_to_local(lua::State *l, const Quat &rLocal, const Quat &r);
		DLLNETWORK Vector3 world_to_local(lua::State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v);
		DLLNETWORK void world_to_local(lua::State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v, const Quat &r);

		DLLNETWORK Vector3 calc_world_direction_from_2d_coordinates(lua::State *l, const Vector3 &forward, const Vector3 &right, const Vector3 &up, float fov, float nearZ, float farZ, float aspectRatio, const Vector2 &uv);
		DLLNETWORK void world_space_point_to_screen_space_uv(lua::State *l, const Vector3 &point, const Mat4 &vp, float nearZ, float farZ);
		DLLNETWORK Vector2 world_space_point_to_screen_space_uv(lua::State *l, const Vector3 &point, const Mat4 &vp);
		DLLNETWORK Vector2 world_space_direction_to_screen_space(lua::State *l, const Vector3 &dir, const Mat4 &vp);
		DLLNETWORK float calc_screenspace_distance_to_worldspace_position(lua::State *l, const Vector3 &point, const Mat4 &vp, float nearZ, float farZ);
		DLLNETWORK float depth_to_distance(lua::State *l, float depth, float nearZ, float farZ);

		DLLNETWORK bool is_same_object(lua::State *l, const luabind::object &o0, const luabind::object &o1);
		DLLNETWORK void clamp_resolution_to_aspect_ratio(lua::State *l, uint32_t w, uint32_t h, float aspectRatio);
		DLLNETWORK void open_url_in_browser(const std::string &url);
		DLLNETWORK void open_path_in_explorer(const std::string &path);
		DLLNETWORK void open_path_in_explorer(const std::string &path, const std::string &selectFile);
		DLLNETWORK std::string get_pretty_bytes(uint32_t bytes);
		DLLNETWORK std::string get_pretty_duration(lua::State *l, uint32_t ms, uint32_t segments, bool noMs);
		DLLNETWORK std::string get_pretty_duration(lua::State *l, uint32_t ms, uint32_t segments);
		DLLNETWORK std::string get_pretty_duration(lua::State *l, uint32_t ms);
		DLLNETWORK std::string get_pretty_time(lua::State *l, float t);
		DLLNETWORK double units_to_metres(double units);
		DLLNETWORK double metres_to_units(double metres);

		DLLNETWORK luabind::object read_scene_file(lua::State *l, const std::string &fileName);

		DLLNETWORK luabind::object fade_property(lua::State *l, LColorProperty &colProp, const Color &colDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LVector2Property &vProp, const Vector2 &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LVector2iProperty &vProp, const Vector2i &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LVector3Property &vProp, const Vector3 &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LVector3iProperty &vProp, const Vector3i &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LVector4Property &vProp, const Vector4 &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LVector4iProperty &vProp, const Vector4i &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LQuatProperty &vProp, const Quat &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LEulerAnglesProperty &vProp, const EulerAngles &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LGenericIntPropertyWrapper &vProp, const int64_t &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua::State *l, LGenericFloatPropertyWrapper &vProp, const double &vDst, float duration);

		DLLNETWORK std::string round_string(lua::State *l, float value, uint32_t places);
		DLLNETWORK std::string round_string(lua::State *l, float value);
		DLLNETWORK std::string get_type_name(lua::State *l, const luabind::object &o);
		DLLNETWORK std::string variable_type_to_string(pragma::util::VarType varType);
		DLLNETWORK std::string get_addon_path(lua::State *l);
		DLLNETWORK std::string get_addon_path(lua::State *l, const std::string &relPath);
		DLLNETWORK std::string get_string_hash(const std::string &str);
		DLLNETWORK luabind::object get_class_value(lua::State *l, const luabind::object &o, const std::string &key);
		DLLNETWORK var<bool, pragma::util::ParallelJob<luabind::object>> pack_zip_archive(lua::State *l, pragma::Game &game, const std::string &zipFileName, const luabind::table<> &t);

		DLLNETWORK void register_std_vector_types(lua::State *l);

		DLLNETWORK void register_library(lua::State *l);

		DLLNETWORK void register_shared_generic(lua::State *l, luabind::module_ &mod);
		DLLNETWORK void register_shared(lua::State *l, luabind::module_ &mod);
		DLLNETWORK void register_world_data(lua::State *l, luabind::module_ &mod);
		DLLNETWORK void register_os(lua::State *l, luabind::module_ &mod);

		DLLNETWORK bool start_debugger_server(lua::State *l);

		struct DLLNETWORK Uuid {
			pragma::util::Uuid value;
		};
	};
};
