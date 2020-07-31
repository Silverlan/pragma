/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LUTIL_H__
#define __LUTIL_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

struct TraceResult;
namespace util {struct SplashDamageInfo;};
namespace Lua
{
	DLLNETWORK void set_ignore_include_cache(bool b);
	namespace global
	{
		DLLNETWORK int include(lua_State *l);
		DLLNETWORK int get_script_path(lua_State *l);

		DLLNETWORK int angle_rand(lua_State *l);
		DLLNETWORK int create_from_string(lua_State *l);
	};
	namespace util
	{
		DLLNETWORK int splash_damage(
			lua_State *l,
			::util::SplashDamageInfo &splashDamageInfo
		);

		DLLNETWORK int is_valid(lua_State *l);
		DLLNETWORK int is_valid_entity(lua_State *l);
		DLLNETWORK int remove(lua_State *l);
		DLLNETWORK int is_table(lua_State *l);
		DLLNETWORK int date_time(lua_State *l);
		DLLNETWORK int fire_bullets(lua_State *l,const std::function<void(::DamageInfo&,::TraceData&,TraceResult&,uint32_t&)> &f);
		DLLNETWORK int fire_bullets(lua_State *l);
		DLLNETWORK int register_class(lua_State *l);

		DLLNETWORK int splash_damage(lua_State *l);
		DLLNETWORK int shake_screen(lua_State *l);
		DLLNETWORK int get_faded_time_factor(lua_State *l);
		DLLNETWORK int get_scale_factor(lua_State *l);

		DLLNETWORK int local_to_world(lua_State *l);
		DLLNETWORK int world_to_local(lua_State *l);

		DLLNETWORK int calc_world_direction_from_2d_coordinates(lua_State *l);

		DLLNETWORK int is_same_object(lua_State *l);
		DLLNETWORK int clamp_resolution_to_aspect_ratio(lua_State *l);
		DLLNETWORK int open_url_in_browser(lua_State *l);
		DLLNETWORK int open_path_in_explorer(lua_State *l);
		DLLNETWORK int get_pretty_bytes(lua_State *l);
		DLLNETWORK int get_pretty_duration(lua_State *l);
		DLLNETWORK int get_pretty_time(lua_State *l);
		DLLNETWORK int units_to_metres(lua_State *l);
		DLLNETWORK int metres_to_units(lua_State *l);

		DLLNETWORK int read_scene_file(lua_State *l);
		DLLNETWORK int fade_property(lua_State *l);
		DLLNETWORK int round_string(lua_State *l);
		DLLNETWORK int get_type_name(lua_State *l);
		DLLNETWORK int variable_type_to_string(lua_State *l);
		DLLNETWORK int get_addon_path(lua_State *l);
		DLLNETWORK int get_string_hash(lua_State *l);
		DLLNETWORK int get_class_value(lua_State *l);
		DLLNETWORK int pack_zip_archive(lua_State *l);
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
	{"get_date_time",Lua::util::date_time}, \
	{"is_table",Lua::util::is_table}, \
	{"is_valid",Lua::util::is_valid}, \
	{"remove",Lua::util::remove}, \
	{"register_class",Lua::util::register_class}, \
	{"get_faded_time_factor",Lua::util::get_faded_time_factor}, \
	{"get_scale_factor",Lua::util::get_scale_factor}, \
	{"local_to_world",Lua::util::local_to_world}, \
	{"world_to_local",Lua::util::world_to_local}, \
	{"get_pretty_bytes",Lua::util::get_pretty_bytes}, \
	{"get_pretty_duration",Lua::util::get_pretty_duration}, \
	{"get_pretty_time",Lua::util::get_pretty_time}, \
	{"units_to_metres",Lua::util::units_to_metres}, \
	{"metres_to_units",Lua::util::metres_to_units}, \
	{"fade_property",Lua::util::fade_property}, \
	{"round_string",Lua::util::round_string}, \
	{"get_type_name",Lua::util::get_type_name}, \
	{"variable_type_to_string",Lua::util::variable_type_to_string}, \
	{"is_same_object",Lua::util::is_same_object}, \
	{"clamp_resolution_to_aspect_ratio",Lua::util::clamp_resolution_to_aspect_ratio}, \
	{"open_url_in_browser",Lua::util::open_url_in_browser}, \
	{"open_path_in_explorer",Lua::util::open_path_in_explorer}, \
	{"get_addon_path",Lua::util::get_addon_path}, \
	{"get_string_hash",Lua::util::get_string_hash}, \
	{"get_class_value",Lua::util::get_class_value}, \
	{"pack_zip_archive",Lua::util::pack_zip_archive},

#define REGISTER_SHARED_UTIL \
	REGISTER_SHARED_UTIL_GENERIC \
	{"is_valid_entity",Lua::util::is_valid_entity}, \
	{"splash_damage",Lua::util::splash_damage}, \
	{"shake_screen",Lua::util::shake_screen}, \
	{"read_scene_file",Lua::util::read_scene_file},

#endif