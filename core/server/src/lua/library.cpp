/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/classes/lresource.h"
#include "pragma/lua/libraries/s_ldebugoverlay.h"
#include "pragma/lua/libraries/s_lai.h"
#include "pragma/lua/libraries/lutil.h"
#include "pragma/lua/libraries/s_lutil.h"
#include "pragma/lua/libraries/s_lsound.h"
#include "pragma/lua/libraries/s_ldebug.h"
#include <pragma/debug/debug_render_info.hpp>
#include <pragma/util/util_splash_damage_info.hpp>
#include <pragma/util/giblet_create_info.hpp>
#include <pragma/util/bulletinfo.h>
#include <pragma/lua/libraries/lasset.hpp>
#include <pragma/lua/luaapi.h>
#include <pragma/lua/classes/lalsound.h>
#include <luainterface.hpp>

void SGame::RegisterLuaLibraries()
{
	Lua::util::register_library(GetLuaState());
	GetLuaInterface().RegisterLibrary("util",{
		REGISTER_SHARED_UTIL
		{"calc_world_direction_from_2d_coordinates",Lua::util::calc_world_direction_from_2d_coordinates}
	});
	auto utilMod = luabind::module(GetLuaState(),"util");
	utilMod[
		luabind::def("fire_bullets",Lua::util::Server::fire_bullets),
		luabind::def("create_giblet",Lua::util::Server::create_giblet),
		luabind::def("create_explosion",Lua::util::Server::create_explosion)
	];

	Game::RegisterLuaLibraries();
	Lua::asset::register_library(GetLuaInterface(),true);

	auto resMod = luabind::module(GetLuaState(),"resource");
	resMod[
		luabind::def("add_file",static_cast<bool(*)(const std::string&,bool)>(Lua::resource::add_file)),
		luabind::def("add_file",static_cast<bool(*)(const std::string&)>(Lua::resource::add_file)),
		luabind::def("add_lua_file",Lua::resource::add_lua_file),
		luabind::def("get_list",Lua::resource::get_list)
	];

	auto utilDebug = luabind::module(GetLuaState(),"debug");
	utilDebug[
		luabind::def("draw_point",Lua::DebugRenderer::Server::DrawPoint),
		luabind::def("draw_line",Lua::DebugRenderer::Server::DrawLine),
		luabind::def("draw_box",&Lua::DebugRenderer::Server::DrawBox),
		luabind::def("draw_sphere",static_cast<void(*)(float,const DebugRenderInfo&)>(Lua::DebugRenderer::Server::DrawSphere)),
		luabind::def("draw_sphere",static_cast<void(*)(float,const DebugRenderInfo&,uint32_t)>(Lua::DebugRenderer::Server::DrawSphere)),
		luabind::def("draw_cone",&Lua::DebugRenderer::Server::DrawCone),
		luabind::def("draw_truncated_cone",&Lua::DebugRenderer::Server::DrawTruncatedCone),
		luabind::def("draw_cylinder",&Lua::DebugRenderer::Server::DrawCylinder),
		luabind::def("draw_pose",&Lua::DebugRenderer::Server::DrawAxis),
		luabind::def("draw_text",static_cast<void(*)(const std::string&,const Vector2&,const DebugRenderInfo&)>(Lua::DebugRenderer::Server::DrawText)),
		luabind::def("draw_text",static_cast<void(*)(const std::string&,float,const DebugRenderInfo&)>(Lua::DebugRenderer::Server::DrawText)),
		luabind::def("draw_path",&Lua::DebugRenderer::Server::DrawPath),
		luabind::def("draw_spline",static_cast<void(*)(lua_State*,luabind::table<>,uint32_t,float,const DebugRenderInfo&)>(Lua::DebugRenderer::Server::DrawSpline)),
		luabind::def("draw_spline",static_cast<void(*)(lua_State*,luabind::table<>,uint32_t,const DebugRenderInfo&)>(Lua::DebugRenderer::Server::DrawSpline)),
		luabind::def("draw_plane",&Lua::DebugRenderer::Server::DrawPlane)
	];

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

	utilDebug[
		luabind::def("behavior_selector_type_to_string",Lua::debug::Server::behavior_selector_type_to_string),
		luabind::def("behavior_task_decorator_type_to_string",Lua::debug::Server::behavior_task_decorator_type_to_string),
		luabind::def("behavior_task_result_to_string",Lua::debug::Server::behavior_task_result_to_string),
		luabind::def("behavior_task_type_to_string",Lua::debug::Server::behavior_task_type_to_string),
		luabind::def("disposition_to_string",Lua::debug::Server::disposition_to_string),
		luabind::def("memory_type_to_string",Lua::debug::Server::memory_type_to_string),
		luabind::def("npc_state_to_string",Lua::debug::Server::npc_state_to_string),
		luabind::def("task_to_string",Lua::debug::Server::task_to_string)
	];
}