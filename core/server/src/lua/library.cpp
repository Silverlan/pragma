/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/classes/lresource.h"
#include "pragma/lua/libraries/s_ldebugoverlay.h"
#include "pragma/lua/libraries/s_lai.h"
#include "pragma/lua/libraries/lutil.h"
#include "pragma/lua/libraries/s_lutil.h"
#include "pragma/lua/libraries/s_lsound.h"
#include "pragma/lua/libraries/s_ldebug.h"
#include <pragma/lua/libraries/lasset.hpp>
#include <pragma/lua/luaapi.h>
#include <pragma/lua/classes/lalsound.h>
#include <luainterface.hpp>

void SGame::RegisterLuaLibraries()
{
	GetLuaInterface().RegisterLibrary("util",{
		REGISTER_SHARED_UTIL
		{"calc_world_direction_from_2d_coordinates",Lua::util::calc_world_direction_from_2d_coordinates},
		{"fire_bullets",Lua::util::Server::fire_bullets},
		{"create_explosion",Lua::util::Server::create_explosion},
		{"create_giblet",Lua::util::Server::create_giblet}
	});

	Game::RegisterLuaLibraries();
	Lua::asset::register_library(GetLuaInterface(),true);

	Lua::RegisterLibrary(GetLuaState(),"resource",{
		{"add_file",Lua::resource::add_file},
		{"add_lua_file",Lua::resource::add_lua_file},
		{"get_list",Lua::resource::get_list}
	});

	std::vector<luaL_Reg> debugFuncs = {
		{"draw_point",&Lua::DebugRenderer::Server::DrawPoint},
		{"draw_line",&Lua::DebugRenderer::Server::DrawLine},
		{"draw_box",&Lua::DebugRenderer::Server::DrawBox},
		{"draw_sphere",&Lua::DebugRenderer::Server::DrawSphere},
		{"draw_cone",&Lua::DebugRenderer::Server::DrawCone},
		{"draw_truncated_cone",&Lua::DebugRenderer::Server::DrawTruncatedCone},
		{"draw_cylinder",&Lua::DebugRenderer::Server::DrawCylinder},
		{"draw_pose",&Lua::DebugRenderer::Server::DrawAxis},
		{"draw_text",&Lua::DebugRenderer::Server::DrawText},
		{"draw_path",&Lua::DebugRenderer::Server::DrawPath},
		{"draw_spline",&Lua::DebugRenderer::Server::DrawSpline},
		{"draw_plane",&Lua::DebugRenderer::Server::DrawPlane}
	};
	for(auto &f : debugFuncs)
	{
		lua_pushtablecfunction(GetLuaState(),"debug",(f.name),(f.func));
	}

	Lua::ai::server::register_library(GetLuaInterface());

	Lua::RegisterLibrary(GetLuaState(),"sound",{
		{"create",Lua::sound::Server::create},
		LUA_LIB_SOUND_SHARED
	});
	Lua::sound::register_enums(GetLuaState());

	auto alSoundClassDef = luabind::class_<ALSound>("Source");
	Lua::ALSound::register_class(alSoundClassDef);

	auto soundMod = luabind::module(GetLuaState(),"sound");
	soundMod[alSoundClassDef];

	lua_pushtablecfunction(GetLuaState(),"debug","behavior_selector_type_to_string",Lua::debug::Server::behavior_selector_type_to_string);
	lua_pushtablecfunction(GetLuaState(),"debug","behavior_task_decorator_type_to_string",Lua::debug::Server::behavior_task_decorator_type_to_string);
	lua_pushtablecfunction(GetLuaState(),"debug","behavior_task_result_to_string",Lua::debug::Server::behavior_task_result_to_string);
	lua_pushtablecfunction(GetLuaState(),"debug","behavior_task_type_to_string",Lua::debug::Server::behavior_task_type_to_string);
	lua_pushtablecfunction(GetLuaState(),"debug","disposition_to_string",Lua::debug::Server::disposition_to_string);
	lua_pushtablecfunction(GetLuaState(),"debug","memory_type_to_string",Lua::debug::Server::memory_type_to_string);
	lua_pushtablecfunction(GetLuaState(),"debug","npc_state_to_string",Lua::debug::Server::npc_state_to_string);
	lua_pushtablecfunction(GetLuaState(),"debug","task_to_string",Lua::debug::Server::task_to_string);
}