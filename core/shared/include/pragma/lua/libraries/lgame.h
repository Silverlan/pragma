/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LGAME_H__
#define __LGAME_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

namespace Lua
{
	namespace game
	{
		// Standard Lua API
		DLLNETWORK int call_callbacks(lua_State *l);

		DLLNETWORK opt<type<CallbackHandle>> add_callback(lua_State *l,const std::string &identifier,const func<void> &function);
		DLLNETWORK void clear_callbacks(lua_State *l,const std::string &identifier);
		DLLNETWORK bool register_ammo_type(lua_State *l,const std::string &name,int32_t damage=10,float force=200.f,DAMAGETYPE damageType=DAMAGETYPE::BULLET);
		DLLNETWORK opt<uint32_t> get_ammo_type_id(lua_State *l,const std::string &name);
		DLLNETWORK opt<std::string> get_ammo_type_name(lua_State *l,uint32_t typeId);
		DLLNETWORK opt<type<pragma::BaseGamemodeComponent>> get_game_mode(lua_State *l);
		DLLNETWORK opt<Vector3> get_light_color(lua_State *l,const Vector3 &pos);
		DLLNETWORK float get_sound_intensity(lua_State *l,const Vector3 &pos);
		DLLNETWORK float get_time_scale(lua_State *l);
		DLLNETWORK void set_time_scale(lua_State *l,float timeScale);
		DLLNETWORK bool is_game_mode_initialized(lua_State *l);
		DLLNETWORK bool raycast(lua_State *l,const ::TraceData &data);;
		DLLNETWORK opt<std::shared_ptr<pragma::nav::Mesh>> get_nav_mesh(lua_State *l);
		DLLNETWORK bool load_nav_mesh(lua_State *l,bool reload=false);
		DLLNETWORK bool is_map_loaded(lua_State *l);
		DLLNETWORK std::string get_map_name(lua_State *l);
		DLLNETWORK Game::GameFlags get_game_state_flags(lua_State *l);
		DLLNETWORK std::pair<bool,int> load_map(lua_State *l,std::string &mapName,BaseEntity **entWorld,Vector3 &origin);

		DLLNETWORK void register_shared_functions(luabind::module_ &modGame);
	};
};

#endif
