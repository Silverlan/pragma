/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUTIL_H__
#define __LUTIL_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

struct TraceResult;
namespace util {struct SplashDamageInfo; enum class VarType : uint8_t;};
namespace Lua
{
	DLLNETWORK void set_ignore_include_cache(bool b);
	namespace global
	{
		DLLNETWORK luabind::object include(lua_State *l,const std::string &f,bool ignoreCache);
		DLLNETWORK luabind::object include(lua_State *l,const std::string &f);
		DLLNETWORK luabind::object exec(lua_State *l,const std::string &f);
		DLLNETWORK std::string get_script_path();

		DLLNETWORK EulerAngles angle_rand();
		DLLNETWORK EulerAngles create_from_string(const std::string &str);
	};
	namespace util
	{
		DLLNETWORK void splash_damage(
			lua_State *l,
			const ::util::SplashDamageInfo &splashDamageInfo
		);

		DLLNETWORK int is_valid(lua_State *l);
		DLLNETWORK int is_valid_entity(lua_State *l);
		DLLNETWORK int remove(lua_State *l);
		DLLNETWORK bool is_table(luabind::argument arg);
		DLLNETWORK bool is_table();
		DLLNETWORK std::string date_time(const std::string &format);
		DLLNETWORK std::string date_time();
		DLLNETWORK int fire_bullets(lua_State *l,const std::function<void(::DamageInfo&,::TraceData&,TraceResult&,uint32_t&)> &f);
		DLLNETWORK int fire_bullets(lua_State *l);
		DLLNETWORK int register_class(lua_State *l);

		DLLNETWORK int shake_screen(lua_State *l);
		DLLNETWORK float get_faded_time_factor(float cur,float dur,float fadeIn,float fadeOut);
		DLLNETWORK float get_faded_time_factor(float cur,float dur,float fadeIn);
		DLLNETWORK float get_faded_time_factor(float cur,float dur);
		DLLNETWORK float get_scale_factor(float val,float min,float max);
		DLLNETWORK float get_scale_factor(float val,float min);

		DLLNETWORK int local_to_world(lua_State *l);
		DLLNETWORK int world_to_local(lua_State *l);

		DLLNETWORK int calc_world_direction_from_2d_coordinates(lua_State *l);
		DLLNETWORK int world_space_point_to_screen_space_uv(lua_State *l);
		DLLNETWORK int world_space_direction_to_screen_space(lua_State *l);
		DLLNETWORK int calc_screenspace_distance_to_worldspace_position(lua_State *l);
		DLLNETWORK int depth_to_distance(lua_State *l);

		DLLNETWORK int is_same_object(lua_State *l);
		DLLNETWORK int clamp_resolution_to_aspect_ratio(lua_State *l);
		DLLNETWORK void open_url_in_browser(const std::string &url);
		DLLNETWORK void open_path_in_explorer(const std::string &path);
		DLLNETWORK void open_path_in_explorer(const std::string &path,const std::string &selectFile);
		DLLNETWORK std::string get_pretty_bytes(uint32_t bytes);
		DLLNETWORK int get_pretty_duration(lua_State *l);
		DLLNETWORK int get_pretty_time(lua_State *l);
		DLLNETWORK double units_to_metres(double units);
		DLLNETWORK double metres_to_units(double metres);

		DLLNETWORK int read_scene_file(lua_State *l);
		DLLNETWORK int fade_property(lua_State *l);
		DLLNETWORK int round_string(lua_State *l);
		DLLNETWORK int get_type_name(lua_State *l);
		DLLNETWORK std::string variable_type_to_string(::util::VarType varType);
		DLLNETWORK std::string get_addon_path(lua_State *l);
		DLLNETWORK std::string get_addon_path(lua_State *l,const std::string &relPath);
		DLLNETWORK std::string get_string_hash(const std::string &str);
		DLLNETWORK int get_class_value(lua_State *l);
		DLLNETWORK int pack_zip_archive(lua_State *l);

		DLLNETWORK void register_std_vector_types(lua_State *l);

		DLLNETWORK void register_library(lua_State *l);
	};

	template<class T,class TCast>
		bool get_table_value(lua_State *l,const std::string &name,uint32_t t,TCast &ret,const std::function<T(lua_State*,int32_t)> &check)
	{
		Lua::PushString(l,name); /* 1 */
		Lua::GetTableValue(l,t);
		auto r = false;
		if(Lua::IsSet(l,-1) == true)
		{
			ret = static_cast<TCast>(check(l,-1));
			r = true;
		}
		Lua::Pop(l,1); /* 0 */
		return r;
	}
	template<typename T>
		void get_table_values(lua_State *l,uint32_t tIdx,std::vector<T> &values,const std::function<T(lua_State*,int32_t)> &tCheck)
	{
		Lua::CheckTable(l,tIdx);
		auto numOffsets = Lua::GetObjectLength(l,tIdx);
		values.reserve(numOffsets);
		for(auto i=decltype(numOffsets){0};i<numOffsets;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,tIdx);

			values.push_back(tCheck(l,-1));
			Lua::Pop(l,1);
		}
	}
	template<typename T>
		std::vector<T> get_table_values(lua_State *l,uint32_t tIdx,const std::function<T(lua_State*,int32_t)> &tCheck)
	{
		std::vector<T> values;
		get_table_values(l,tIdx,values,tCheck);
		return values;
	}
};

#define REGISTER_SHARED_UTIL_GENERIC \
	{"is_valid",Lua::util::is_valid}, \
	{"remove",Lua::util::remove}, \
	{"register_class",Lua::util::register_class}, \
	{"local_to_world",Lua::util::local_to_world}, \
	{"world_to_local",Lua::util::world_to_local}, \
	{"get_pretty_duration",Lua::util::get_pretty_duration}, \
	{"get_pretty_time",Lua::util::get_pretty_time}, \
	{"fade_property",Lua::util::fade_property}, \
	{"round_string",Lua::util::round_string}, \
	{"get_type_name",Lua::util::get_type_name}, \
	{"is_same_object",Lua::util::is_same_object}, \
	{"clamp_resolution_to_aspect_ratio",Lua::util::clamp_resolution_to_aspect_ratio}, \
	{"get_class_value",Lua::util::get_class_value}, \
	{"pack_zip_archive",Lua::util::pack_zip_archive}, \
	{"world_space_point_to_screen_space_uv",Lua::util::world_space_point_to_screen_space_uv}, \
	{"world_space_direction_to_screen_space",Lua::util::world_space_direction_to_screen_space}, \
	{"calc_screen_space_distance_to_world_space_position",Lua::util::calc_screenspace_distance_to_worldspace_position}, \
	{"depth_to_distance",Lua::util::depth_to_distance},

#define REGISTER_SHARED_UTIL \
	REGISTER_SHARED_UTIL_GENERIC \
	{"is_valid_entity",Lua::util::is_valid_entity}, \
	{"shake_screen",Lua::util::shake_screen}, \
	{"read_scene_file",Lua::util::read_scene_file},

#endif