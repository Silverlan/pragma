/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUTIL_HPP__
#define __LUTIL_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/classes/lproperty.hpp"
#include <pragma/lua/luaapi.h>
#include <mathutil/uvec.h>

struct TraceResult;
struct BulletInfo;
namespace util {
	struct SplashDamageInfo;
	enum class VarType : uint8_t;
};
namespace pragma {
	class BaseFlexComponent;
};
namespace Lua {
	namespace global {
		DLLNETWORK luabind::object include(lua_State *l, const std::string &f, std::vector<std::string> *optCache, bool reload, bool throwErr);
		DLLNETWORK luabind::object include(lua_State *l, const std::string &f, std::vector<std::string> *optCache, bool reload);
		DLLNETWORK luabind::object include(lua_State *l, const std::string &f, std::vector<std::string> *optCache);
		DLLNETWORK luabind::object include(lua_State *l, const std::string &f, bool ignoreGlobalCache);
		DLLNETWORK luabind::object include(lua_State *l, const std::string &f);
		DLLNETWORK luabind::object exec(lua_State *l, const std::string &f);
		DLLNETWORK std::string get_script_path();

		DLLNETWORK EulerAngles angle_rand();
		DLLNETWORK EulerAngles create_from_string(const std::string &str);
	};
	namespace util {
		DLLNETWORK void splash_damage(lua_State *l, const ::util::SplashDamageInfo &splashDamageInfo);

		DLLNETWORK bool is_valid(lua_State *l);
		DLLNETWORK bool is_valid(lua_State *l, const luabind::object &o);
		DLLNETWORK bool is_valid_entity(lua_State *l);
		DLLNETWORK bool is_valid_entity(lua_State *l, const luabind::object &o);
		DLLNETWORK void remove(lua_State *l, const luabind::object &o);
		DLLNETWORK void remove(lua_State *l, const luabind::object &o, bool removeSafely);
		DLLNETWORK bool is_table(luabind::argument arg);
		DLLNETWORK bool is_table();
		DLLNETWORK std::string date_time(const std::string &format);
		DLLNETWORK std::string date_time();
		DLLNETWORK luabind::object fire_bullets(lua_State *l, BulletInfo &bulletInfo, bool hitReport, const std::function<void(::DamageInfo &, ::TraceData &, TraceResult &, uint32_t &)> &f);
		DLLNETWORK luabind::object fire_bullets(lua_State *l, BulletInfo &bulletInfo, bool hitReport);
		DLLNETWORK luabind::object fire_bullets(lua_State *l, BulletInfo &bulletInfo);

		DLLNETWORK luabind::object register_class(lua_State *l, const std::string &className);
		DLLNETWORK luabind::object register_class(lua_State *l, const std::string &className, const luabind::object &oBase0);
		DLLNETWORK luabind::object register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1);
		DLLNETWORK luabind::object register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2);
		DLLNETWORK luabind::object register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3);
		DLLNETWORK luabind::object register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3, const luabind::object &oBase4);
		DLLNETWORK luabind::object register_class(lua_State *l, const luabind::table<> &t, const std::string &className);
		DLLNETWORK luabind::object register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0);
		DLLNETWORK luabind::object register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1);
		DLLNETWORK luabind::object register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2);
		DLLNETWORK luabind::object register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3);
		DLLNETWORK luabind::object register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3, const luabind::object &oBase4);

		DLLNETWORK void shake_screen(lua_State *l, const Vector3 &pos, float radius, float amplitude, float frequency, float duration, float fadeIn, float fadeOut);
		DLLNETWORK void shake_screen(lua_State *l, float amplitude, float frequency, float duration, float fadeIn, float fadeOut);
		DLLNETWORK float get_faded_time_factor(float cur, float dur, float fadeIn, float fadeOut);
		DLLNETWORK float get_faded_time_factor(float cur, float dur, float fadeIn);
		DLLNETWORK float get_faded_time_factor(float cur, float dur);
		DLLNETWORK float get_scale_factor(float val, float min, float max);
		DLLNETWORK float get_scale_factor(float val, float min);

		DLLNETWORK Quat local_to_world(lua_State *l, const Quat &r0, const Quat &r1);
		DLLNETWORK Vector3 local_to_world(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v);
		DLLNETWORK void local_to_world(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v, const Quat &r);

		DLLNETWORK Quat world_to_local(lua_State *l, const Quat &rLocal, const Quat &r);
		DLLNETWORK Vector3 world_to_local(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v);
		DLLNETWORK void world_to_local(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v, const Quat &r);

		DLLNETWORK Vector3 calc_world_direction_from_2d_coordinates(lua_State *l, const Vector3 &forward, const Vector3 &right, const Vector3 &up, float fov, float nearZ, float farZ, float aspectRatio, const ::Vector2 &uv);
		DLLNETWORK void world_space_point_to_screen_space_uv(lua_State *l, const Vector3 &point, const ::Mat4 &vp, float nearZ, float farZ);
		DLLNETWORK ::Vector2 world_space_point_to_screen_space_uv(lua_State *l, const Vector3 &point, const ::Mat4 &vp);
		DLLNETWORK ::Vector2 world_space_direction_to_screen_space(lua_State *l, const Vector3 &dir, const ::Mat4 &vp);
		DLLNETWORK float calc_screenspace_distance_to_worldspace_position(lua_State *l, const Vector3 &point, const ::Mat4 &vp, float nearZ, float farZ);
		DLLNETWORK float depth_to_distance(lua_State *l, float depth, float nearZ, float farZ);

		DLLNETWORK bool is_same_object(lua_State *l, const luabind::object &o0, const luabind::object &o1);
		DLLNETWORK void clamp_resolution_to_aspect_ratio(lua_State *l, uint32_t w, uint32_t h, float aspectRatio);
		DLLNETWORK void open_url_in_browser(const std::string &url);
		DLLNETWORK void open_path_in_explorer(const std::string &path);
		DLLNETWORK void open_path_in_explorer(const std::string &path, const std::string &selectFile);
		DLLNETWORK std::string get_pretty_bytes(uint32_t bytes);
		DLLNETWORK std::string get_pretty_duration(lua_State *l, uint32_t ms, uint32_t segments, bool noMs);
		DLLNETWORK std::string get_pretty_duration(lua_State *l, uint32_t ms, uint32_t segments);
		DLLNETWORK std::string get_pretty_duration(lua_State *l, uint32_t ms);
		DLLNETWORK std::string get_pretty_time(lua_State *l, float t);
		DLLNETWORK double units_to_metres(double units);
		DLLNETWORK double metres_to_units(double metres);

		DLLNETWORK luabind::object read_scene_file(lua_State *l, const std::string &fileName);

		DLLNETWORK luabind::object fade_property(lua_State *l, LColorProperty &colProp, const Color &colDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LVector2Property &vProp, const ::Vector2 &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LVector2iProperty &vProp, const ::Vector2i &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LVector3Property &vProp, const ::Vector3 &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LVector3iProperty &vProp, const ::Vector3i &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LVector4Property &vProp, const ::Vector4 &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LVector4iProperty &vProp, const ::Vector4i &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LQuatProperty &vProp, const ::Quat &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LEulerAnglesProperty &vProp, const ::EulerAngles &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LGenericIntPropertyWrapper &vProp, const int64_t &vDst, float duration);
		DLLNETWORK luabind::object fade_property(lua_State *l, LGenericFloatPropertyWrapper &vProp, const double &vDst, float duration);

		DLLNETWORK std::string round_string(lua_State *l, float value, uint32_t places);
		DLLNETWORK std::string round_string(lua_State *l, float value);
		DLLNETWORK std::string get_type_name(lua_State *l, const luabind::object &o);
		DLLNETWORK std::string variable_type_to_string(::util::VarType varType);
		DLLNETWORK std::string get_addon_path(lua_State *l);
		DLLNETWORK std::string get_addon_path(lua_State *l, const std::string &relPath);
		DLLNETWORK std::string get_string_hash(const std::string &str);
		DLLNETWORK luabind::object get_class_value(lua_State *l, const luabind::object &o, const std::string &key);
		DLLNETWORK void pack_zip_archive(lua_State *l, const std::string &zipFileName, const luabind::table<> &t);

		DLLNETWORK void register_std_vector_types(lua_State *l);

		DLLNETWORK void register_library(lua_State *l);

		DLLNETWORK void register_shared_generic(luabind::module_ &mod);
		DLLNETWORK void register_shared(luabind::module_ &mod);

		struct DLLNETWORK Uuid {
			::util::Uuid value;
		};

		struct DLLNETWORK TranslationData {
			BoneId boneIdOther;
			umath::Transform transform;
		};

		namespace retarget {
			struct DLLNETWORK RetargetData {
				std::vector<umath::ScaledTransform> absBonePoses;
				std::vector<umath::ScaledTransform> origBindPoseToRetargetBindPose;
				std::vector<float> origBindPoseBoneDistances;

				std::vector<umath::ScaledTransform> bindPosesOther;
				std::vector<umath::ScaledTransform> origBindPoses;
				std::vector<umath::ScaledTransform> tmpPoses;
				std::vector<umath::ScaledTransform> retargetPoses;
				umath::Transform invRootPose;

				std::vector<umath::Transform> bindPoseTransforms;
				std::vector<umath::ScaledTransform> relBindPoses;
				std::unordered_set<BoneId> untranslatedBones;

				std::unordered_map<BoneId, TranslationData> translationTable;
			};
			std::shared_ptr<RetargetData> initialize_retarget_data(luabind::object absBonePoses, luabind::object origBindPoseToRetargetBindPose, luabind::object origBindPoseBoneDistances,

			  luabind::object bindPosesOther, luabind::object origBindPoses, luabind::object tmpPoses, luabind::object retargetPoses, luabind::object invRootPose,

			  luabind::object bindPoseTransforms, luabind::object relBindPoses,

			  luabind::object tUntranslatedBones, luabind::object tTranslationTable);
			void apply_retarget_rig(RetargetData &testData, Model &mdl, pragma::BaseAnimatedComponent &animSrc, pragma::BaseAnimatedComponent &animDst, panima::Skeleton &skeleton);

			struct DLLNETWORK RetargetFlexData {
				struct RemapData {
					float minSource;
					float maxSource;

					float minTarget;
					float maxTarget;
				};
				std::unordered_map<uint32_t, std::unordered_map<uint32_t, RemapData>> remapData;
			};
			std::shared_ptr<RetargetFlexData> initialize_retarget_flex_data(luabind::object remapData);
			void retarget_flex_controllers(RetargetFlexData &retargetFlexData, pragma::BaseFlexComponent &flexCSrc, pragma::BaseFlexComponent &flexCDst);
		};
	};
};

#endif
